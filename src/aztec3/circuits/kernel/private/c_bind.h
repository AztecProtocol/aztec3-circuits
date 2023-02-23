#include <cstdint>
#include "init.hpp"

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void private_kernel__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output);

}
