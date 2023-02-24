#include "c_bind.h"
#include "private_kernel_circuit.hpp"
#include <aztec3/circuits/abis/tx_request.hpp>

#include <common/streams.hpp>
#include <common/mem.hpp>
#include <common/container.hpp>
#include <cstdint>
#include <ecc/curves/grumpkin/grumpkin.hpp>
#include <crypto/pedersen_commitment/pedersen.hpp>
#include <sstream>

using namespace barretenberg;
using namespace aztec3::circuits::abis;
using namespace aztec3::circuits::kernel::private_kernel;

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
WASM_EXPORT void private_kernel__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output)
{
    TxRequest<NT> tx_request;
    read(tx_request_buf, tx_request);
    NT::fr::serialize_to_buffer(tx_request.hash(), output);
}
}
