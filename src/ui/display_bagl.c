#ifdef HAVE_BAGL

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdbool.h>  // bool
#include <stdio.h>    // snprintf
#include <string.h>   // memset
#include <stdint.h>

#include "os.h"
#include "ux.h"

#include "./display.h"
#include "./display_utils.h"
#include "../constants.h"
#include "../globals.h"
#include "../boilerplate/io.h"
#include "../boilerplate/sw.h"
#include "../common/bip32.h"
#include "../common/format.h"
#include "../common/script.h"
#include "../constants.h"

/*
    STATELESS STEPS
    As these steps do not access per-step globals (except possibly a callback), they can be used in
   any flow.
*/

// Step with icon and text for pubkey
UX_STEP_NOCB(ux_display_confirm_pubkey_step, pn, {&C_icon_eye, "Confirm public key"});

// Step with icon and text for a suspicious address
UX_STEP_NOCB(ux_display_unusual_derivation_path_step,
             pnn,
             {
                 &C_icon_warning,
                 "The derivation",
                 "path is unusual",
             });

// Step with icon and text to caution the user to reject if unsure
UX_STEP_CB(ux_display_reject_if_not_sure_step,
           pnn,
           set_ux_flow_response(false),
           {
               &C_icon_crossmark,
               "Reject if you're",
               "not sure",
           });

// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           set_ux_flow_response(true),
           {
               &C_icon_validate_14,
               "Approve",
           });

// Step with continue button
UX_STEP_CB(ux_display_continue_step,
           pb,
           set_ux_flow_response(true),
           {
               &C_icon_validate_14,
               "Continue",
           });

// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           set_ux_flow_response(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

/*
    STATEFUL STEPS
    These can only be used in the context of specific flows, as they access a common shared space
   for strings.
*/

// PATH/PUBKEY or PATH/ADDRESS

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = g_ui_state.path_and_pubkey.bip32_path_str,
             });

// Step with title/text for pubkey
UX_STEP_NOCB(ux_display_pubkey_step,
             bnnn_paging,
             {
                 .title = "Public key",
                 .text = g_ui_state.path_and_pubkey.pubkey,
             });

// Step with description of a wallet policy
UX_STEP_NOCB(ux_display_wallet_policy_map_step,
             bnnn_paging,
             {
                 .title = "Wallet policy:",
                 .text = g_ui_state.wallet.descriptor_template,
             });

// Step with index and xpub of a cosigner of a policy_map wallet
UX_STEP_NOCB(ux_display_wallet_policy_cosigner_pubkey_step,
             bnnn_paging,
             {
                 .title = g_ui_state.cosigner_pubkey_and_index.signer_index,
                 .text = g_ui_state.cosigner_pubkey_and_index.pubkey,
             });

// Step with title/text for address, used when showing a wallet receive address
UX_STEP_NOCB(ux_display_wallet_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_ui_state.wallet.address,
             });

// Step with warning icon and text explaining that there are external inputs
UX_STEP_NOCB(ux_display_warning_external_inputs_step,
             pnn,
             {
                 &C_icon_warning,
                 "There are",
                 "external inputs",
             });

// Step with warning icon for unverified inputs (segwit inputs with no non-witness-utxo)
UX_STEP_NOCB(ux_unverified_segwit_input_flow_1_step, pb, {&C_icon_warning, "Unverified inputs"});
UX_STEP_NOCB(ux_unverified_segwit_input_flow_2_step, nn, {"Update", "Ledger Live"});
UX_STEP_NOCB(ux_unverified_segwit_input_flow_3_step, nn, {"or third party", "wallet software"});

// Step with warning icon for nondefault sighash
UX_STEP_NOCB(ux_nondefault_sighash_flow_1_step, pb, {&C_icon_warning, "Non-default sighash"});

// Step with eye icon and "Review" and the output index
UX_STEP_NOCB(ux_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 g_ui_state.validate_output.index,
             });

// Step with "Amount" and an output amount
UX_STEP_NOCB(ux_validate_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_ui_state.validate_output.amount,
             });

// Step with "Address" and a paginated address
UX_STEP_NOCB(ux_validate_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_ui_state.validate_output.address_or_description,
             });

// Step with eye icon and a "high fees" warning
UX_STEP_NOCB(ux_high_fee_step,
             pnn,
             {
                 &C_icon_eye,
                 "Fees are above 10%",
                 "of total amount!",
             });

UX_STEP_NOCB(ux_confirm_selftransfer_step, pnn, {&C_icon_eye, "Confirm", "self-transfer"});
UX_STEP_NOCB(ux_confirm_transaction_fees_step,
             bnnn_paging,
             {
                 .title = "Fees",
                 .text = g_ui_state.validate_transaction.fee,
             });
UX_STEP_CB(ux_sign_transaction_step,
           pbb,
           set_ux_flow_response(true),
           {&C_icon_validate_14, "Sign", "transaction"});

// Step with wallet icon and "Register account"
UX_STEP_NOCB(ux_display_register_wallet_step,
             pb,
             {
                 &C_icon_wallet,
                 "Register account",
             });

// Step with wallet icon and "Receive in known wallet"
UX_STEP_NOCB(ux_display_receive_in_registered_wallet_step,
             pnn,
             {
                 &C_icon_wallet,
                 "Receive in",
                 "known account",
             });

// Step with wallet icon and "Spend from known wallet"
UX_STEP_NOCB(ux_display_spend_from_registered_wallet_step,
             pnn,
             {
                 &C_icon_wallet,
                 "Spend from",
                 "known account",
             });

// Step with "Wallet name:", followed by the wallet name
UX_STEP_NOCB(ux_display_wallet_name_step,
             bnnn_paging,
             {
                 .title = "Account name:",
                 .text = g_ui_state.wallet.wallet_name,
             });

//////////////////////////////////////////////////////////////////////
UX_STEP_NOCB(ux_sign_message_step,
             pnn,
             {
                 &C_icon_certificate,
                 "Sign",
                 "message",
             });

UX_STEP_CB(ux_message_sign_display_path_step,
           bnnn_paging,
           set_ux_flow_response(true),
           {
               .title = "Path",
               .text = g_ui_state.path_and_message.bip32_path_str,
           });

UX_STEP_NOCB(ux_message_hash_step,
             bnnn_paging,
             {
                 .title = "Message hash",
                 .text = g_ui_state.path_and_message.message,
             });

UX_STEP_CB(ux_sign_message_accept_new,
           pbb,
           set_ux_flow_response(true),
           {&C_icon_validate_14, "Sign", "message"});

UX_STEP_CB(ux_message_content_step,
           custom,
           set_ux_flow_response(true),
           {
               .title = "Message content",
               .text = g_ui_state.path_and_message.message,
           });

//////////////////////////////////////////////////////////////////////
UX_STEP_NOCB(ux_withdraw_step,
             pnn,
             {
                 &C_icon_certificate,
                 "Withdraw",
                 "",
             });

UX_STEP_NOCB(ux_withdraw_display_value_step,
             bnnn_paging,
             {
                 .title = "Value",
                 .text = g_ui_state.validate_withdraw.value,
             });

UX_STEP_NOCB(ux_withdraw_display_redeemer_address_step,
             bnnn_paging,
             {
                 .title = "Redeemer Address",
                 .text = g_ui_state.validate_withdraw.redeemer_address,
             });

UX_STEP_CB(ux_withdraw_accept_step,
           pbb,
           set_ux_flow_response(true),
           {&C_icon_validate_14, "Approve", "withdraw"});

//////////////////////////////////////////////////////////////////////
UX_STEP_NOCB(ux_display_erc4361_step,
             pnn,
             {
                 &C_icon_certificate,
                 "Sign In",
                 "",
             });

UX_STEP_NOCB(ux_display_erc4361_domain_step,
             bnnn_paging,
             {
                 .title = "Domain",
                 .text = g_ui_state.validate_erc4361.domain,
             });

UX_STEP_NOCB(ux_display_erc4361_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_ui_state.validate_erc4361.address,
             });
UX_STEP_NOCB(ux_display_erc4361_uri_step,
             bnnn_paging,
             {
                 .title = "URI",
                 .text = g_ui_state.validate_erc4361.uri,
             });
UX_STEP_NOCB(ux_display_erc4361_version_step,
             bnnn_paging,
             {
                 .title = "Version",
                 .text = g_ui_state.validate_erc4361.version,
             });
UX_STEP_NOCB(ux_display_erc4361_nonce_step,
             bnnn_paging,
             {
                 .title = "Nonce",
                 .text = g_ui_state.validate_erc4361.nonce,
             });
UX_STEP_NOCB(ux_display_erc4361_issued_at_step,
             bnnn_paging,
             {
                 .title = "Issued at",
                 .text = g_ui_state.validate_erc4361.issued_at,
             });
UX_STEP_NOCB(ux_display_erc4361_expiration_time_step,
             bnnn_paging,
             {
                 .title = "Expiration time",
                 .text = g_ui_state.validate_erc4361.expiration_time,
             });

// FLOW to display BIP32 path to sign a message:
// #1 screen: certificate icon + "Sign message"
// #2 screen: display BIP32 Path
// #3 screen: first page of message content
UX_FLOW(ux_sign_message_path_and_content_flow,
        &ux_sign_message_step,
        &ux_message_sign_display_path_step,
        &ux_message_content_step,
        &ux_message_content_step);

// FLOW to display a message hash and confirmation to sign a message:
// #1 screen: certificate icon + "Sign message"
// #2 screen: display BIP32 Path
// #3 screen: display message hash
// #4 screen: "Sign message" and approve button
// #5 screen: reject button
UX_FLOW(ux_sign_message_path_hash_and_confirm_flow,
        &ux_sign_message_step,
        &ux_message_sign_display_path_step,
        &ux_message_hash_step,
        &ux_sign_message_accept_new,
        &ux_display_reject_step);

// FLOW to display the message content:
// #1 screen: display message content
UX_FLOW(ux_sign_message_content_flow, &ux_message_content_step, FLOW_LOOP);

// FLOW to display a confirmation to sign a message:
// #1 screen: "Sign message" and approve button
// #2 screen: reject button
UX_FLOW(ux_sign_message_confirm_flow, &ux_sign_message_accept_new, &ux_display_reject_step);

// FLOW to display BIP32 path and pubkey:
// #1 screen: eye icon + "Confirm Pubkey"
// #2 screen: display BIP32 Path
// #3 screen: display pubkey
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_pubkey_step,
        &ux_display_path_step,
        &ux_display_pubkey_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display BIP32 path and pubkey, for a non-standard path:
// #1 screen: warning icon + "The derivation path is unusual"
// #2 screen: crossmark icon + "Reject if not sure" (user can reject here)
// #3 screen: eye icon + "Confirm Pubkey"
// #4 screen: display BIP32 Path
// #5 screen: display pubkey
// #6 screen: approve button
// #7 screen: reject button
UX_FLOW(ux_display_pubkey_suspicious_flow,
        &ux_display_unusual_derivation_path_step,
        &ux_display_confirm_pubkey_step,
        &ux_display_path_step,
        &ux_display_reject_if_not_sure_step,
        &ux_display_pubkey_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display the header of a policy map wallet:
// #1 screen: Wallet icon + "Register account"
// #2 screen: "Account name:" and wallet policy name
// #3 screen: display policy map (paginated)
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_register_wallet_flow,
        &ux_display_register_wallet_step,
        &ux_display_wallet_name_step,
        &ux_display_wallet_policy_map_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display the header of a policy_map wallet:
// #1 screen: Cosigner index and pubkey (paginated)
// #2 screen: approve button
// #3 screen: reject button
UX_FLOW(ux_display_policy_map_cosigner_pubkey_flow,
        &ux_display_wallet_policy_cosigner_pubkey_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display the name and an address of a registered wallet:
// #1 screen: Wallet icon + "Receive in known wallet"
// #2 screen: wallet name
// #3 screen: wallet address (paginated)
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_receive_in_wallet_flow,
        &ux_display_receive_in_registered_wallet_step,
        &ux_display_wallet_name_step,
        &ux_display_wallet_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display an address of a default wallet policy:
// #1 screen: wallet address (paginated)
// #2 screen: approve button
// #3 screen: reject button
UX_FLOW(ux_display_default_wallet_address_flow,
        &ux_display_wallet_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

// FLOW to display a registered wallet and authorize spending:
// #1 screen: "Spend from known wallet"
// #2 screen: wallet name
// #3 screen: "Continue" button
// #4 screen: reject button
UX_FLOW(ux_display_spend_from_wallet_flow,
        &ux_display_spend_from_registered_wallet_step,
        &ux_display_wallet_name_step,
        &ux_display_continue_step,
        &ux_display_reject_step);

// FLOW to warn about external inputs
// #1 screen: warning icon + "There are external inputs"
// #2 screen: crossmark icon + "Reject if not sure" (user can reject here)
// #3 screen: "continue" button
UX_FLOW(ux_display_warning_external_inputs_flow,
        &ux_display_warning_external_inputs_step,
        &ux_display_reject_if_not_sure_step,
        &ux_display_continue_step);

// FLOW to warn about segwitv0 inputs with no non-witness-utxo
// #1 screen: warning icon + "Unverified inputs"
// #2 screen: "Update Ledger Live"
// #3 screen: "or external wallet software"
// #4 screen: "continue" button
// #5 screen: "reject" button
UX_FLOW(ux_display_unverified_segwit_inputs_flow,
        &ux_unverified_segwit_input_flow_1_step,
        &ux_unverified_segwit_input_flow_2_step,
        &ux_unverified_segwit_input_flow_3_step,
        &ux_display_continue_step,
        &ux_display_reject_step);

// FLOW to warn about segwitv1 inputs with non-default sighash
// #1 screen: warning icon + "Non default sighash"
// #2 screen: crossmark icon + "Reject if not sure" (user can reject here)
// #3 screen: "continue" button
// #4 screen: "reject" button
UX_FLOW(ux_display_nondefault_sighash_flow,
        &ux_nondefault_sighash_flow_1_step,
        &ux_display_reject_if_not_sure_step,
        &ux_display_continue_step,
        &ux_display_reject_step);

// FLOW to validate a single output
// #1 screen: eye icon + "Review" + index of output to validate
// #2 screen: output amount
// #3 screen: output address (paginated)
// #4 screen: "Continue" button
// #5 screen: reject button
UX_FLOW(ux_display_output_address_amount_flow,
        &ux_review_step,
        &ux_validate_amount_step,
        &ux_validate_address_step,
        &ux_display_continue_step,
        &ux_display_reject_step);

// FLOW to warn about fees above 10% of total
// #1 screen: eye icon + fees too high message
// #2 screen: "Continue", with approve button
// #3 screen: reject button
UX_FLOW(ux_warn_high_fee_flow,
        &ux_high_fee_step,
        &ux_display_continue_step,
        &ux_display_reject_step);

// Show transaction fees and finally accept signing
// #1 screen: fee amount
// #2 screen: "Sign transaction", with approve button
// #3 screen: reject button
UX_FLOW(ux_accept_transaction_flow,
        &ux_confirm_transaction_fees_step,
        &ux_sign_transaction_step,
        &ux_display_reject_step);

// Show transaction fees and finally accept signing, in case of self-transfer
// Since there is no output to show, we add an initial screen to make sure
// the user understands the nature of the transaction.
// #1 screen: eye icon + "Confirm self-transfer"
// #2 screen: fee amount
// #3 screen: "Sign transaction", with approve button
// #4 screen: reject button
UX_FLOW(ux_accept_selftransfer_flow,
        &ux_confirm_selftransfer_step,
        &ux_confirm_transaction_fees_step,
        &ux_sign_transaction_step,
        &ux_display_reject_step);

// FLOW to data from withdraw tx:
// #1 screen: certificate icon + "Withdraw"
// #2 screen: value
// #3 screen: redeemer_address
// #4 screen: "Approve" button
// #5 screen: "Reject" button
UX_FLOW(ux_withdraw_display_data_flow,
        &ux_withdraw_step,
        &ux_withdraw_display_value_step,
        &ux_withdraw_display_redeemer_address_step,
        &ux_withdraw_accept_step,
        &ux_display_reject_step);

// FLOW to display ERC4361:
// #1 screen: certificate icon + "Sign ERC-4361"
// #2 screen: display Domain
// #3 screen: display Address
// #4 screen: display URI
// #5 screen: display Version
// #6 screen: display Nonce
// #7 screen: display Issued at
// #8 screen: display Expiration time
// #9 screen: "Approve" button
// #10 screen: "Reject" button
UX_FLOW(ux_display_erc4361_content_flow,
        &ux_display_erc4361_step,
        &ux_display_erc4361_domain_step,
        &ux_display_erc4361_address_step,
        &ux_display_erc4361_uri_step,
        &ux_display_erc4361_version_step,
        &ux_display_erc4361_nonce_step,
        &ux_display_erc4361_issued_at_step,
        &ux_display_erc4361_expiration_time_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

void ui_display_pubkey_flow(void) {
    ux_flow_init(0, ux_display_pubkey_flow, NULL);
}

void ui_display_pubkey_suspicious_flow(void) {
    ux_flow_init(0, ux_display_pubkey_suspicious_flow, NULL);
}

void ui_sign_message_path_hash_and_confirm_flow(void) {
    ux_flow_init(0, ux_sign_message_path_hash_and_confirm_flow, NULL);
}

void ui_sign_message_content_flow(void) {
    if (get_streaming_index() == 0) {
        ux_flow_init(0, ux_sign_message_path_and_content_flow, NULL);
    } else {
        ux_flow_init(0, ux_sign_message_content_flow, NULL);
    }
}

void ui_sign_message_confirm_flow(void) {
    ux_flow_init(0, ux_sign_message_confirm_flow, NULL);
}

void ui_display_register_wallet_flow(void) {
    ux_flow_init(0, ux_display_register_wallet_flow, NULL);
}

void ui_display_policy_map_cosigner_pubkey_flow(void) {
    ux_flow_init(0, ux_display_policy_map_cosigner_pubkey_flow, NULL);
}

void ui_display_receive_in_wallet_flow(void) {
    ux_flow_init(0, ux_display_receive_in_wallet_flow, NULL);
}

void ui_display_default_wallet_address_flow(void) {
    ux_flow_init(0, ux_display_default_wallet_address_flow, NULL);
}

void ui_display_spend_from_wallet_flow(void) {
    ux_flow_init(0, ux_display_spend_from_wallet_flow, NULL);
}

void ui_display_warning_external_inputs_flow(void) {
    ux_flow_init(0, ux_display_warning_external_inputs_flow, NULL);
}

void ui_display_unverified_segwit_inputs_flows(void) {
    ux_flow_init(0, ux_display_unverified_segwit_inputs_flow, NULL);
}

void ui_display_nondefault_sighash_flow(void) {
    ux_flow_init(0, ux_display_nondefault_sighash_flow, NULL);
}

void ui_display_output_address_amount_flow(int index) {
    snprintf(g_ui_state.validate_output.index,
             sizeof(g_ui_state.validate_output.index),
             "output #%d",
             index);
    ux_flow_init(0, ux_display_output_address_amount_flow, NULL);
}

void ui_display_output_address_amount_no_index_flow(int index) {
    // Currently we don't want any change in the UX so this function defaults to
    // ui_display_output_address_amount_flow
    ui_display_output_address_amount_flow(index);
}

void ui_warn_high_fee_flow(void) {
    ux_flow_init(0, ux_warn_high_fee_flow, NULL);
}

void ui_accept_transaction_flow(bool is_self_transfer) {
    ux_flow_init(0,
                 is_self_transfer ? ux_accept_selftransfer_flow : ux_accept_transaction_flow,
                 NULL);
}

void ui_display_withdraw_content_flow(void) {
    ux_flow_init(0, ux_withdraw_display_data_flow, NULL);
}

void ui_display_erc4361_content_flow(void) {
    ux_flow_init(0, ux_display_erc4361_content_flow, NULL);
}

#endif  // HAVE_BAGL
