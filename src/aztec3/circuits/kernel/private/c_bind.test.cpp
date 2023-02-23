#include "c_bind.h"

#include <aztec3/circuits/abis/call_context.hpp>
#include <aztec3/circuits/abis/contract_deployment_data.hpp>
#include <aztec3/circuits/abis/function_signature.hpp>
#include <aztec3/circuits/abis/tx_context.hpp>
#include <aztec3/circuits/abis/tx_request.hpp>
#include <aztec3/circuits/abis/private_circuit_public_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/public_inputs.hpp>

#include <aztec3/circuits/apps/function_execution_context.hpp>
#include <aztec3/circuits/apps/test_apps/escrow/deposit.hpp>

#include <common/test.hpp>
#include <gtest/gtest.h>

namespace {

using aztec3::circuits::abis::CallContext;
using aztec3::circuits::abis::ContractDeploymentData;
using aztec3::circuits::abis::FunctionSignature;
using aztec3::circuits::abis::OptionalPrivateCircuitPublicInputs;
using aztec3::circuits::abis::PrivateCircuitPublicInputs;
using aztec3::circuits::abis::TxContext;
using aztec3::circuits::abis::TxRequest;

using aztec3::circuits::apps::FunctionExecutionContext;
using aztec3::circuits::apps::test_apps::escrow::deposit;

} // namespace

namespace aztec3::circuits::kernel::private_kernel {

TEST(private_kernel, hash_tx_request)
{
    // Most of this is just setup (creating TxRequest to then be hashed)
    const NT::address escrow_contract_address = 12345;

    const NT::fr msg_sender_private_key = 123456789;
    const NT::address msg_sender =
        NT::fr(uint256_t(0x01071e9a23e0f7edULL, 0x5d77b35d1830fa3eULL, 0xc6ba3660bb1f0c0bULL, 0x2ef9f7f09867fd6eULL));
    const NT::address tx_origin = msg_sender;

    Composer deposit_composer = Composer("../barretenberg/cpp/srs_db/ignition");
    DB db;

    FunctionSignature<NT> function_signature{
        .function_encoding = 1, // TODO: deduce this from the contract, somehow.
        .is_private = true,
        .is_constructor = false,
    };

    CallContext<NT> call_context{
        .msg_sender = msg_sender,
        .storage_contract_address = escrow_contract_address,
        .tx_origin = msg_sender,
        .is_delegate_call = false,
        .is_static_call = false,
        .is_contract_deployment = false,
        .reference_block_num = 0,
    };

    NativeOracle deposit_oracle =
        NativeOracle(db, escrow_contract_address, function_signature, call_context, msg_sender_private_key);
    OracleWrapper deposit_oracle_wrapper = OracleWrapper(deposit_composer, deposit_oracle);

    FunctionExecutionContext deposit_ctx(deposit_composer, deposit_oracle_wrapper);

    auto amount = NT::fr(5);
    auto asset_id = NT::fr(1);
    auto memo = NT::fr(999);

    OptionalPrivateCircuitPublicInputs<NT> opt_deposit_public_inputs = deposit(deposit_ctx, amount, asset_id, memo);
    PrivateCircuitPublicInputs<NT> deposit_public_inputs = opt_deposit_public_inputs.remove_optionality();

    TxRequest<NT> tx_request = TxRequest<NT>{
        .from = tx_origin,
        .to = escrow_contract_address,
        .function_signature = function_signature,
        .args = deposit_public_inputs.args,
        .nonce = 0,
        .tx_context =
            TxContext<NT>{
                .is_fee_payment_tx = false,
                .is_rebate_payment_tx = false,
                .is_contract_deployment_tx = false,
                .contract_deployment_data = ContractDeploymentData<NT>(),
                .reference_block_num = 0,
            },
        .chain_id = 1,
    };

    // Perform c_bind hash and check result
    std::vector<uint8_t> buf;
    write(buf, tx_request);
    uint8_t* output = (uint8_t*)malloc(sizeof(uint8_t) * 32);
    private_kernel__hash_tx_request(buf.data(), output);

    NT::fr hash = NT::fr::serialize_from_buffer(output);

    std::string got_hash = format(hash);
    std::string expected_hash = "0x0fb3576464a061dd1322b9d47a18f470d1a4405e076a20f86c87bfdcd59c789f";

    info("Got hash: ", got_hash);
    info("Expected hash: ", expected_hash);
    EXPECT_EQ(got_hash, expected_hash);
}

} // namespace aztec3::circuits::kernel::private_kernel