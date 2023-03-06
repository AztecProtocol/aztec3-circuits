#include "c_bind.h"

#include "tx_request.hpp"

#include <numeric/random/engine.hpp>
#include <gtest/gtest.h>

namespace {

using NT = plonk::stdlib::types::NativeTypes;
auto& engine = numeric::random::get_debug_engine();

/**
 * @details Convert a bytes buffer to a hex string.
 *
 * @param bytes buffer of bytes to be converted to hex string
 * @param first_n_bytes only include the first n bytes of `bytes` in the conversion
 * @return a string containing the hex representation of the first n bytes of the input buffer
 */
std::string bytes_to_hex_str(uint8_t* bytes, int first_n_bytes)
{
    std::ostringstream stream;
    for (int i = 0; i < first_n_bytes; i++) {
        stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytes[i]);
    }
    return stream.str();
}

} // namespace

namespace aztec3::circuits::abis {

TEST(abis, hash_tx_request)
{
    // randomize function args for tx request
    std::array<fr, ARGS_LENGTH> args;
    for (size_t i = 0; i < ARGS_LENGTH; i++) {
        args[i] = fr(engine.get_random_uint256());
    }

    // Construct mostly empty TxRequest with some randomized fields
    TxRequest<NT> tx_request = TxRequest<NT>{
        .from = engine.get_random_uint256(),
        .to = engine.get_random_uint256(),
        .function_signature = FunctionSignature<NT>(),
        .args = args,
        .nonce = engine.get_random_uint256(),
        .tx_context = TxContext<NT>(),
        .chain_id = engine.get_random_uint256(),
    };

    // Write the tx request to a buffer and
    // allocate an output buffer for cbind hash results
    std::vector<uint8_t> buf;
    write(buf, tx_request);
    uint8_t* output = (uint8_t*)malloc(32 * sizeof(uint8_t));
    // Make the c_bind call to hash the tx request
    abis__hash_tx_request(buf.data(), output);

    // Convert buffer to `fr` for comparison to in-test calculated hash
    NT::fr got_hash = NT::fr::serialize_from_buffer(output);
    // Confirm cbind output == hash of tx request
    EXPECT_EQ(got_hash, tx_request.hash());
}

TEST(abis, compute_function_selector_keccak256_transfer)
{
    const char* function_signature = "transfer(address,uint256)";

    // allocate an output buffer for cbind selector results
    uint8_t* output = (uint8_t*)malloc(4 * sizeof(uint8_t));
    // Make the c_bind call to compute the function selector
    abis__compute_function_selector_keccak256(function_signature, output);

    // get the selector as a hex string of 4 bytes and
    // compare against known good selector from solidity
    EXPECT_EQ(bytes_to_hex_str(output, 4), "a9059cbb");
}

TEST(abis, compute_function_selector_keccak256_transferFrom)
{
    const char* function_signature = "transferFrom(address,address,uint256)";

    // allocate an output buffer for cbind selector results
    uint8_t* output = (uint8_t*)malloc(4 * sizeof(uint8_t));
    // Make the c_bind call to compute the function selector
    abis__compute_function_selector_keccak256(function_signature, output);

    // get the selector as a hex string of 4 bytes and
    // compare against known good selector from solidity
    EXPECT_EQ(bytes_to_hex_str(output, 4), "23b872dd");
}

TEST(abis, compute_function_selector_pedersen_transfer)
{
    const char* function_signature = "transfer(address,uint256)";

    // allocate an output buffer for cbind selector results
    uint8_t* output = (uint8_t*)malloc(4 * sizeof(uint8_t));
    // Make the c_bind call to compute the function selector
    abis__compute_function_selector_pedersen(function_signature, output);

    // get the selector as a hex string of 4 bytes
    EXPECT_EQ(bytes_to_hex_str(output, 4), "15eee575");
}

TEST(abis, compute_function_selector_pedersen_transferFrom)
{
    const char* function_signature = "transferFrom(address,address,uint256)";

    // allocate an output buffer for cbind selector results
    uint8_t* output = (uint8_t*)malloc(4 * sizeof(uint8_t));
    // Make the c_bind call to compute the function selector
    abis__compute_function_selector_pedersen(function_signature, output);

    // get the selector as a hex string of 4 bytes
    EXPECT_EQ(bytes_to_hex_str(output, 4), "06db4c02");
}

} // namespace aztec3::circuits::abis
