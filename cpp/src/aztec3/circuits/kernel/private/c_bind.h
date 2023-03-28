#include <cstdint>
#include <cstddef>

#define WASM_EXPORT __attribute__((visibility("default")))

extern "C" {

WASM_EXPORT size_t private_kernel__init_proving_key(uint8_t const** pk_buf);
WASM_EXPORT size_t private_kernel__init_verification_key(uint8_t const* pk_buf, uint8_t const** vk_buf);
//WASM_EXPORT size_t private_kernel__get_default_previous_kernel(uint8_t const** previous_kernel_data);
WASM_EXPORT size_t private_kernel__create_proof(uint8_t const* signed_tx_request_buf,
                                                uint8_t const* previous_kernel_buf,
                                                uint8_t const* private_call_buf,
                                                uint8_t const* pk_buf,
                                                bool proverless,
                                                uint8_t const** proof_data_buf,
                                                size_t* proof_data_size,
                                                uint8_t const** private_kernel_public_inputs_buf);
WASM_EXPORT size_t private_kernel__verify_proof(uint8_t const* vk_buf,
                                                uint8_t const* proof,
                                                uint32_t length);

}