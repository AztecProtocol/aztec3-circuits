#include <cstdint>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT void abis__hash_tx_request(uint8_t const* tx_request_buf, uint8_t* output);
WASM_EXPORT void abis__compute_function_selector(char const* func_sig_cstr, uint8_t* output);
WASM_EXPORT void abis__compute_function_leaf(uint8_t const* function_selector_buf, bool is_private, uint8_t const* vk_hash_buf, uint8_t const* acir_hash_buf, uint8_t* output);

}
