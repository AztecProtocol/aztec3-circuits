#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void abis__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output);
WASM_EXPORT void abis__compute_function_selector_keccak256(char const* func_sig_buf, uint8_t* output);
WASM_EXPORT void abis__compute_function_selector_pedersen(char const* func_sig_buf, uint8_t* output);

}
