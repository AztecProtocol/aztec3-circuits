#include <gtest/gtest.h>
#include <barretenberg/common/test.hpp>
#include <barretenberg/common/serialize.hpp>
#include "index.hpp"
#include <barretenberg/stdlib/types/types.hpp>

namespace aztec3::circuits::abis {

using Composer = plonk::stdlib::types::Composer;
using CT = aztec3::utils::types::CircuitTypes<Composer>;
using NT = aztec3::utils::types::NativeTypes;

class abi_tests : public ::testing::Test {};

TEST(abi_tests, test_native_function_data)
{
    FunctionData<NT> function_data = {
        .function_selector = 11,
        .is_private = false,
        .is_constructor = false,
    };

    info("function data: ", function_data);

    auto buffer = to_buffer(function_data);
    auto function_data_2 = from_buffer<FunctionData<NT>>(buffer.data());

    EXPECT_EQ(function_data, function_data_2);
}

TEST(abi_tests, test_native_to_circuit_function_data)
{
    FunctionData<NT> native_function_data = {
        .function_selector = 11,
        .is_private = false,
        .is_constructor = false,
    };

    info("function data: ", native_function_data);

    Composer composer = Composer("../barretenberg/cpp/srs_db/ignition");
    FunctionData<CT> circuit_function_data = native_function_data.to_circuit_type(composer);

    info("function data: ", circuit_function_data);
}

TEST(abi_tests, test_native_call_context)
{
    CallContext<NT> call_context = {
        .msg_sender = 10,
        .storage_contract_address = 11,
        .tx_origin = 12,
        .is_delegate_call = false,
        .is_static_call = false,
    };

    info("call context: ", call_context);
}

TEST(abi_tests, test_native_to_circuit_call_context)
{
    CallContext<NT> native_call_context = {
        .msg_sender = 10,
        .storage_contract_address = 11,
        .tx_origin = 12,
        .is_delegate_call = false,
        .is_static_call = false,
    };

    info("call context: ", native_call_context);

    Composer composer = Composer("../barretenberg/cpp/srs_db/ignition");
    CallContext<CT> circuit_call_context = native_call_context.to_circuit_type(composer);

    info("call context: ", circuit_call_context);
}

} // namespace aztec3::circuits::abis