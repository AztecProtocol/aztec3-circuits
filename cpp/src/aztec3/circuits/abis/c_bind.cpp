#include "c_bind.h"
#include "tx_request.hpp"

#include <stdlib/types/native_types.hpp>
#include <crypto/keccak/keccak.hpp>
#include <common/serialize.hpp>

namespace {
using aztec3::GeneratorIndex;
using aztec3::circuits::abis::TxRequest;
using NT = plonk::stdlib::types::NativeTypes;
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
WASM_EXPORT void abis__compute_function_selector(char const* func_sig_cstr, uint8_t* output)
{
    // hash the function signature using keccak256
    auto keccak_hash = ethash_keccak256(reinterpret_cast<uint8_t const*>(func_sig_cstr), strlen(func_sig_cstr));
    // get a pointer to the start of the hash bytes
    uint8_t const* hash_bytes = reinterpret_cast<uint8_t const*>(&keccak_hash.word64s[0]);
    // get the first 4 bytes of the hash's 0th word and copy into output buffer
    std::copy(hash_bytes, hash_bytes + 4, output);
}

#define FUNC_SEL_BYTES 32
#define VK_HASH_BYTES 32
#define ACIR_HASH_BYTES 32
#define FUNC_LEAF_PREIMAGE_BYTES FUNC_SEL_BYTES + 1 + VK_HASH_BYTES + ACIR_HASH_BYTES

#define FUNC_SEL_OFFSET 0
#define IS_PRIVATE_OFFSET FUNC_SEL_BYTES
#define VK_HASH_OFFSET IS_PRIVATE_OFFSET + 1
#define ACIR_HASH_OFFSET VK_HASH_OFFSET + ACIR_HASH_BYTES

WASM_EXPORT void abis__compute_function_leaf(uint8_t const* function_selector_buf,
                                             bool is_private,
                                             uint8_t const* vk_hash_buf,
                                             uint8_t const* acir_hash_buf,
                                             uint8_t* output)
{
    // info("Sizeof fs_buf: ", sizeof(function_selector_buf));
    // uint32_t size;
    // serialize::read(function_selector_buf, size);
    // info("Read sizeof fs_buf: ", size);

    // TODO somehow ensure that function selector is 4 bytes and vk_hash is 32 (actually 31?)
    std::vector<uint8_t> to_compress(FUNC_LEAF_PREIMAGE_BYTES, 0);
    std::copy_n(function_selector_buf, FUNC_SEL_BYTES, to_compress.begin());
    to_compress[IS_PRIVATE_OFFSET] = (uint8_t)is_private;
    std::copy_n(vk_hash_buf, VK_HASH_BYTES, to_compress.begin() + VK_HASH_OFFSET);
    std::copy_n(acir_hash_buf, ACIR_HASH_BYTES, to_compress.begin() + ACIR_HASH_OFFSET);

    info("Function leaf preimage: ", to_compress);
    NT::fr leaf = NT::compress(to_compress, GeneratorIndex::FUNCTION_LEAF);

    info("HERE6 leaf: ", leaf);
    NT::fr::serialize_to_buffer(leaf, output);
    info("HERE7");
}
}
