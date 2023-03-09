#include "c_bind.h"

#include "tx_request.hpp"

#include <numeric/random/engine.hpp>
#include <gtest/gtest.h>

namespace {

using NT = plonk::stdlib::types::NativeTypes;
auto& engine = numeric::random::get_debug_engine();

/**
 * @brief Convert a bytes array to a hex string.
 *
 * @details convert each byte to two hex characters
 *
 * @tparam NUM_BYTES length of bytes array input
 * @param bytes array of bytes to be converted to hex string
 * @return a string containing the hex representation of the NUM_BYTES bytes of the input array
 */
template <size_t NUM_BYTES> std::string bytes_to_hex_str(std::array<uint8_t, NUM_BYTES> bytes)
{
    std::ostringstream stream;
    for (const uint8_t& byte : bytes) {
        stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
    }
    return stream.str();
}

/**
 * @brief Convert a hex string to a bytes array
 *
 * @details convert each hex pair to a byte and return array of bytes
 *
 * @tparam NUM_BYTES length of bytes array output. May be different from length of hex string.
 * @param hex_str the hex string to be converted to bytes. The number of bytes that will
 * be inserted into the output array is `min(NUM_BYTES, ceil(hex_str.length()/2))`.
 * @return an array containing the bytes representation of the hex string's character pairs.
 */
template <size_t NUM_BYTES> std::array<uint8_t, NUM_BYTES> hex_str_to_bytes(std::string& hex_str)
{
    // if NUM_BYTES is less than the bytes in the hex string, we want to cut the loop short
    size_t const num_bytes_to_process = std::min<size_t>(NUM_BYTES, (size_t)ceil((double)hex_str.length() / 2));

    std::array<uint8_t, NUM_BYTES> bytes = { 0 };
    for (size_t i = 0; i < num_bytes_to_process * 2; i += 2) {
        std::string byteString = hex_str.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        bytes[i / 2] = byte;
    }
    return bytes;
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

    // create an output buffer for cbind hash results
    std::array<uint8_t, 32> output = { 0 };
    // Make the c_bind call to hash the tx request
    abis__hash_tx_request(buf.data(), output.data());

    // Convert buffer to `fr` for comparison to in-test calculated hash
    NT::fr got_hash = NT::fr::serialize_from_buffer(output.data());

    // Confirm cbind output == hash of tx request
    EXPECT_EQ(got_hash, tx_request.hash());
}

TEST(abis, compute_function_selector_transfer)
{
    const char* function_signature = "transfer(address,uint256)";

    // create an output buffer for cbind selector results
    std::array<uint8_t, 4> output = { 0 };
    // Make the c_bind call to compute the function selector via keccak256
    abis__compute_function_selector(function_signature, output.data());

    // get the selector as a hex string of 4 bytes and
    // compare against known good selector from solidity
    EXPECT_EQ(bytes_to_hex_str(output), "a9059cbb");
}

TEST(abis, compute_function_selector_transferFrom)
{
    const char* function_signature = "transferFrom(address,address,uint256)";

    // create an output buffer for cbind selector results
    std::array<uint8_t, 4> output = { 0 };
    // Make the c_bind call to compute the function selector via keccak256
    abis__compute_function_selector(function_signature, output.data());

    // get the selector as a hex string of 4 bytes and
    // compare against known good selector from solidity
    EXPECT_EQ(bytes_to_hex_str(output), "23b872dd");
}

TEST(abis, compute_function_leaf)
{
    info("Sizeof fr type: ", sizeof(NT::fr));
    info("Sizeof fr: ", sizeof(NT::fr(0)));
    // need to compress function_selector, is_private, vk_hash, acir_hash
    std::string function_selector_str = "a9059cbb";
    bool is_private = true;
    std::string vk_hash_str = "123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
    std::string acir_hash_str = "89ABCDEF89ABCDEF89ABCDEF89ABCDEF89ABCDEF89ABCDEF89ABCDEF89ABCDEF";

    // 0th byte is a9, 3rd byte is bb
    std::array<uint8_t, 32> fs_bytes = hex_str_to_bytes<32>(function_selector_str);
    // 0th byte is 0x12, 31st byte is 0xF, last few bits are omitted since fields are only 254 bits
    std::array<uint8_t, 32> vk_hash_bytes = hex_str_to_bytes<32>(vk_hash_str);
    // 0th byte is 0x89, 31st byte is 0xF, last few bits are omitted since fields are only 254 bits
    std::array<uint8_t, 32> acir_hash_bytes = hex_str_to_bytes<32>(acir_hash_str);

    // FIXME should this be 31 instead of 32? Since fields only have 254?
    // NOTE: convert_buffer_to_field assumes 31 bytes per field
    // printf("fs_bytes_buf[0]: %u\n", (uint8_t)fs_bytes.data()[0]);
    // printf("fs_bytes_buf[1]: %u\n", (uint8_t)fs_bytes.data()[1]);
    // printf("fs_bytes_buf[2]: %u\n", (uint8_t)fs_bytes.data()[2]);
    // printf("fs_bytes_buf[3]: %u\n", (uint8_t)fs_bytes.data()[3]);

    // std::string fs_hex_str = bytes_to_hex_str(fs_bytes.data(), 4);
    // info("fs: ", fs_hex_str);
    // info("fs[0]: ", fs_hex_str[0]);

    std::array<uint8_t, 32> output = { 0 };
    abis__compute_function_leaf(
        fs_bytes.data(), is_private, vk_hash_bytes.data(), acir_hash_bytes.data(), output.data());

    NT::fr got_leaf = NT::fr::serialize_from_buffer(output.data());

    info("leaf ", got_leaf);
}

} // namespace aztec3::circuits::abis
