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

WASM_EXPORT void private_kernel__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output)
{
    TxRequest<NT> tx_request;
    read(tx_request_buf, tx_request);
    barretenberg::fr::serialize_to_buffer(tx_request.hash(), output);
}
}
