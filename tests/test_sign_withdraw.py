import pytest

from ledger_bitcoin.exception.errors import DenyError, IncorrectDataError
from ledger_bitcoin.exception.device_exception import DeviceException
from ragger.navigator import Navigator
from ragger.firmware import Firmware
from ragger.error import ExceptionRAPDU
from ragger_bitcoin import RaggerClient
from ledger_bitcoin.withdraw import AcreWithdrawalData
from .instructions import withdrawal_instruction_approve, withdrawal_instruction_reject

#
def test_sign_withdraw(navigator: Navigator, firmware: Firmware, client: RaggerClient, test_name: str):
    data = AcreWithdrawalData(
        to= "0xc14972DC5a4443E4f5e89E3655BE48Ee95A795aB",
        value= "0x0",
        data= "0xcae9ca510000000000000000000000007e184179b1F95A9ca398E6a16127f06b81Cb37a3000000000000000000000000000000000000000000000000002386F26FC10000000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000001000000000000000000000000006083Bde64CCBF08470a1a0dAa9a0281B4951be7C4fa8f3322330a4be2d34fdd2a573eaa5f94c7fe5000000000000000000000000d2f85a52fee8ee905e504f75dcf34156b2503004de1079ddaeceaf643b38a034000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005835b17e900000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000001a1976a914c8e9edf5e915c0482b1b236fc917011a4b943e6e88ac000000000000",
        operation= "0",
        safeTxGas= "0x0",
        baseGas= "0x0",
        gasPrice= "0x0",
        gasToken= "0x0000000000000000000000000000000000000000",
        refundReceiver= "0x0000000000000000000000000000000000000000",
        nonce= "0x8",
    ) # tx_hash: 0xa580c0c5a0b8e731f7b7fe8ea3ee41cf0dbe78b352b55ae090160659e2d7410c
    path = "m/44'/0'/0'/0/0"
    result = client.sign_withdraw(data, path, navigator,
                                 instructions=withdrawal_instruction_approve(firmware),
                                 testname=test_name)
    assert result == "Hy2UpLBXRUkHBRfXIEYFB8PEteLtjxrqJ7kJ3Qe+i67wP0bzDkFl5Z4bYBFfT/3+xwgPrw3T0rkq6dv53Cff+p0="

def test_sign_withdraw_wrong_address(navigator: Navigator, firmware: Firmware, client: RaggerClient, test_name: str):
    data = AcreWithdrawalData(
        to= "0xc14972DC5a4443E4f5e89E3655BE48Ee95A795aB",
        value= "0x0",
        data= "0xcae9ca510000000000000000000000000e781e9d538895ee99bd6e9bf28664942beff32f00000000000000000000000000000000000000000000000000470de4df820000000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000001000000000000000000000000006083Bde64CCBF08470a1a0dAa9a0281B4951be7C4b5e4623765ec95cfa6e261406d5c446012eff9300000000000000000000000008dcc842b8ed75efe1f222ebdc22d1b06ef35efff6469f708057266816f0595200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000587f579c500000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000001a1976a9143c6480044cfafde6dad7f718f76938cc87d0679a88ac000000000000",
        operation= "0",
        safeTxGas= "0x0",
        baseGas= "0x0",
        gasPrice= "0x0",
        gasToken= "0x0000000000000000000000000000000000000000",
        refundReceiver= "0x0000000000000000000000000000000000000000",
        nonce= "0xC",
    )
    path = "m/44'/0'/0'/0/0"
    with pytest.raises(ExceptionRAPDU) as e:
        result = client.sign_withdraw(data, path, navigator,
                                    instructions=withdrawal_instruction_approve(firmware),
                                    testname=test_name)

        assert DeviceException.exc.get(e.value.status) == IncorrectDataError
        assert len(e.value.data) == 0

def test_sign_withdraw_reject(navigator: Navigator, firmware: Firmware, client: RaggerClient, test_name: str):
    data = AcreWithdrawalData(
        to= "0xc14972DC5a4443E4f5e89E3655BE48Ee95A795aB",
        value= "0x0",
        data= "0xcae9ca510000000000000000000000000e781e9d538895ee99bd6e9bf28664942beff32f00000000000000000000000000000000000000000000000000470de4df820000000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000001000000000000000000000000006083Bde64CCBF08470a1a0dAa9a0281B4951be7C4b5e4623765ec95cfa6e261406d5c446012eff9300000000000000000000000008dcc842b8ed75efe1f222ebdc22d1b06ef35efff6469f708057266816f0595200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000587f579c500000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000001a1976a9143c6480044cfafde6dad7f718f76938cc87d0679a88ac000000000000",
        operation= "0",
        safeTxGas= "0x0",
        baseGas= "0x0",
        gasPrice= "0x0",
        gasToken= "0x0000000000000000000000000000000000000000",
        refundReceiver= "0x0000000000000000000000000000000000000000",
        nonce= "0x8",
    )
    path = "m/44'/0'/0'/0/0"
    with pytest.raises(ExceptionRAPDU) as e:
        client.sign_withdraw(data, path, navigator,
                                    instructions=withdrawal_instruction_reject(firmware),
                                    testname=test_name)

        assert DeviceException.exc.get(e.value.status) == DenyError
        assert len(e.value.data) == 0
