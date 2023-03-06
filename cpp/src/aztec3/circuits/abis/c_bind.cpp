#include "c_bind.h"
#include "tx_request.hpp"

#include <stdlib/types/native_types.hpp>
#include <crypto/keccak/keccak.hpp>

namespace {

using aztec3::GeneratorIndex;
using aztec3::circuits::abis::TxRequest;
using NT = plonk::stdlib::types::NativeTypes;

// 31 bytes since field is really 254
const size_t FR_USABLE_BYTES = sizeof(NT::fr) - 1;

/**
 * @brief Converts a C-string to a vector of fr/fields
 *
 * @details given a `char const*` cstr, determine how many fields will
 * be necessary to hold all of the string's characters. Allow `FR_USABLE_BYTES`
 * bytes per fr/field. Initialize a 0-valued uint256 for each one, then memcpy the
 * string's bytes into the uint256, and pass that to the fr constructor.
 * Insert each fr into a vector<fr> and return that.
 *
 * @param cstr the C-string to convert to an fr vector
 * @return a vector of frs/fields that contain the bytes from the C-string arg padded with 0s.
 */
std::vector<NT::fr> cstr_to_fr_vec(char const* cstr)
{
    // Determine how many frs/fields will be necessary to store all of the string's bytes
    std::string str = cstr;
    const auto num_frs_for_str = (size_t)ceil((double)str.length() / FR_USABLE_BYTES);

    // Calculate how many non-zero there will be in the last (possibly not full) field
    const auto remainder_bytes = str.length() % FR_USABLE_BYTES;
    const auto bytes_for_last_fr = (remainder_bytes == 0) ? FR_USABLE_BYTES : remainder_bytes;

    std::vector<NT::fr> vec;
    for (size_t i = 0; i < num_frs_for_str; i++) {
        // The length of the substring that maps to this fr depends on
        // whether this the last fr being constructed. If so, use the previously
        // calculated `bytes_for_last_fr`, otherwise this fr needs no padding so use `FR_USABLE_BYTES`
        size_t substr_len = (i == num_frs_for_str - 1) ? bytes_for_last_fr : FR_USABLE_BYTES;

        // Offset into the relevant bytes of the cstr argument for this iteration (this fr)
        size_t str_offset = i * FR_USABLE_BYTES;
        // initialize a full 256 bits of zeros (this will include any necessary padding for
        // substr_len < FR_USABLE_BYTES)
        uint256_t substr_as_uint256(0); // initialize fr to all zeros
        // copy substr's bytes into this uint256
        memcpy(&substr_as_uint256, cstr + str_offset, substr_len);

        // use this uint256 to initialize an fr
        NT::fr substr_as_fr(substr_as_uint256);

        vec.insert(vec.end(), (NT::fr)substr_as_fr);
    }
    return vec;
}

} // namespace

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

/**
 * @brief Hashes a TX request. This is a WASM-export that can be called from Typescript.
 *
 * @details given a `uint8_t*` buffer representing a full TX request,
 * read it into a `TxRequest` object, hash it to a `fr`,
 * and serialize it to a `uint8_t*` output buffer
 *
 * @param tx_request_buf buffer of bytes containing all data needed to construct a TX request via `read()`
 * @param output buffer that will contain the output which will be the hashed `TxRequest`
 */
WASM_EXPORT void abis__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output)
{
    TxRequest<NT> tx_request;
    read(tx_request_buf, tx_request);
    NT::fr::serialize_to_buffer(tx_request.hash(), output);
}

/**
 * @brief Generates a function's "selector" from its "signature" using keccak256.
 * This is a WASM-export that can be called from Typescript.
 *
 * @details given a `char const*` c-string representing a "function signature",
 * hash using keccak and return its first 4 bytes (the "function selector")
 * by copying them into the `output` buffer arg. This is a workalike of
 * Ethereum/solidity's function selector computation....
 * Ethereum function selector is computed as follows:
 * `uint8_t* hash = keccak256(const char* func_sig);`
 * where func_sig does NOT include the trailing null character
 * And the resulting cstring for "transfer(address,uint256)" is:
 * `0xa9059cbb`
 * The 0th to 3rd bytes make up the function selector like:
 * where 0xa9 is hash[0], 05 is hash[1], 9c is hash[2], and bb is hash[3]
 *
 * @param func_sig_cstr c-string representing the function signature string like "transfer(uint256,address)"
 * @param output buffer that will contain the output which will be 4-byte function selector
 */
WASM_EXPORT void abis__compute_function_selector_keccak256(char const* func_sig_cstr, uint8_t* output)
{
    // Hash the function signature using keccak256
    auto keccak_hash = ethash_keccak256((uint8_t*)func_sig_cstr, std::char_traits<char>::length(func_sig_cstr));
    // Get the first 4 bytes of the hash's 0th word and copy into output buffer
    memcpy(output, &keccak_hash.word64s[0], 4 * sizeof(uint8_t));
}

/**
 * @brief Generates a function's "selector" from its "signature" using pedersen.
 * This is a WASM-export that can be called from Typescript.
 *
 * @details given a `char const*` c-string representing a "function signature",
 * convert to a vector of frs/fields (packing into as few fields as possible)
 * hash to a fr/field, convert it to a `uint8_t*` buffer,
 * and return its first 4 bytes (the "function selector")
 * by copying them into the `output` buffer arg. This is similar to
 * Ethereum/solidity's function selector computation except it uses pedersen
 * instead of keccak256.
 *
 * @param func_sig_cstr c-string representing the function signature string like "transfer(uint256,address)"
 * @param output buffer that will contain the output which will be 4-byte function selector
 */
WASM_EXPORT void abis__compute_function_selector_pedersen(char const* func_sig_cstr, uint8_t* output)
{
    // Convert function signature cstr to a packed+padded vector of fields
    std::vector<NT::fr> func_sig_vec = cstr_to_fr_vec(func_sig_cstr);

    // Hash it to a field using pedersen
    NT::fr func_sig_hash = NT::compress(func_sig_vec, GeneratorIndex::FUNCTION_SELECTOR);

    // Convert the field to a bytes buffer
    std::vector<uint8_t> hash_bytes = func_sig_hash.to_buffer();

    // Copy the raw data, but only the first bytes which define comprise the "selector"
    memcpy(output, hash_bytes.data(), 4 * sizeof(uint8_t));
}
}
