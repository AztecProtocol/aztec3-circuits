#include "index.hpp"
#include "init.hpp"
#include "c_bind.h"

// TODO remove these? present in init/index?
#include <aztec3/constants.hpp>
#include <aztec3/utils/types/native_types.hpp>
#include "aztec3/circuits/abis/signed_tx_request.hpp"
#include "aztec3/circuits/abis/private_kernel/private_call_data.hpp"
#include <aztec3/circuits/abis/private_kernel/private_inputs.hpp>
#include <aztec3/circuits/abis/private_kernel/public_inputs.hpp>
#include <aztec3/circuits/mock/mock_kernel_circuit.hpp>

#include "barretenberg/srs/reference_string/env_reference_string.hpp"

#include "barretenberg/common/serialize.hpp"
#include "barretenberg/plonk/composer/turbo_composer.hpp"

namespace {
using NT = aztec3::utils::types::NativeTypes;
using aztec3::circuits::abis::SignedTxRequest;
using aztec3::circuits::abis::private_kernel::PreviousKernelData;
using aztec3::circuits::abis::private_kernel::PrivateCallData;
using aztec3::circuits::abis::private_kernel::PrivateInputs;
using aztec3::circuits::abis::private_kernel::PublicInputs;
using aztec3::circuits::kernel::private_kernel::native_private_kernel_circuit;
using aztec3::circuits::kernel::private_kernel::private_kernel_circuit;
using aztec3::circuits::mock::mock_kernel_circuit;

using plonk::TurboComposer;
using namespace plonk::stdlib::types;

PreviousKernelData<NT> default_previous_kernel()
{
    // TODO confirm this is the right way to initialize struct of 0s
    auto mock_kernel_public_inputs = PublicInputs<NT>();
    mock_kernel_public_inputs.end.private_call_stack =
        std::array<NT::fr, aztec3::KERNEL_PRIVATE_CALL_STACK_LENGTH>({ { 0 } }),
    mock_kernel_public_inputs.is_private = true;

    auto crs_factory = std::make_shared<EnvReferenceStringFactory>();
    Composer mock_kernel_composer = Composer(crs_factory);
    mock_kernel_circuit(mock_kernel_composer, mock_kernel_public_inputs);

    plonk::stdlib::types::Prover mock_kernel_prover = mock_kernel_composer.create_prover();
    NT::Proof mock_kernel_proof = mock_kernel_prover.construct_proof();

    std::shared_ptr<NT::VK> mock_kernel_vk = mock_kernel_composer.compute_verification_key();

    PreviousKernelData<NT> previous_kernel = {
        .public_inputs = mock_kernel_public_inputs,
        .proof = mock_kernel_proof,
        .vk = mock_kernel_vk,
    };
    return previous_kernel;
}
} // namespace

#define WASM_EXPORT __attribute__((visibility("default")))
// WASM Cbinds
extern "C" {

WASM_EXPORT size_t private_kernel__init_proving_key(uint8_t const** pk_buf)
{
    std::vector<uint8_t> pk_vec(42, 0);

    auto raw_buf = (uint8_t*)malloc(pk_vec.size());
    memcpy(raw_buf, (void*)pk_vec.data(), pk_vec.size());
    *pk_buf = raw_buf;

    return pk_vec.size();
}

WASM_EXPORT size_t private_kernel__init_verification_key(uint8_t const* pk_buf, uint8_t const** vk_buf)
{
    // TODO actual verification key
    // NT:VKData vk_data = { 0 };

    std::vector<uint8_t> vk_vec(42, 0);
    // write(vk_vec, vk_data);
    info(pk_buf);

    auto raw_buf = (uint8_t*)malloc(vk_vec.size());
    memcpy(raw_buf, (void*)vk_vec.data(), vk_vec.size());
    *vk_buf = raw_buf;

    return vk_vec.size();
}

// TODO comment about how public_inputs is a confusing name
// returns size of public inputs
WASM_EXPORT size_t private_kernel__sim(uint8_t const* signed_tx_request_buf,
                                       uint8_t const* previous_kernel_buf,
                                       uint8_t const* private_call_buf,
                                       bool first_iteration,
                                       uint8_t const** private_kernel_public_inputs_buf)
{
    SignedTxRequest<NT> signed_tx_request;
    read(signed_tx_request_buf, signed_tx_request);

    PrivateCallData<NT> private_call_data;
    read(private_call_buf, private_call_data);

    PreviousKernelData<NT> previous_kernel;
    if (first_iteration) {
        previous_kernel = default_previous_kernel();
    } else {
        read(previous_kernel_buf, previous_kernel);
    }

    PrivateInputs<NT> private_inputs = PrivateInputs<NT>{
        .signed_tx_request = signed_tx_request,
        .previous_kernel = default_previous_kernel(),
        .private_call = private_call_data,

    };

    PublicInputs<NT> public_inputs = native_private_kernel_circuit(private_inputs);

    // serialize public inputs to bytes vec
    std::vector<uint8_t> public_inputs_vec;
    write(public_inputs_vec, public_inputs);
    // copy public inputs to output buffer
    auto raw_public_inputs_buf = (uint8_t*)malloc(public_inputs_vec.size());
    memcpy(raw_public_inputs_buf, (void*)public_inputs_vec.data(), public_inputs_vec.size());
    *private_kernel_public_inputs_buf = raw_public_inputs_buf;

    return public_inputs_vec.size();
}

// returns size of proof data
WASM_EXPORT size_t private_kernel__prove(uint8_t const* signed_tx_request_buf,
                                         uint8_t const* previous_kernel_buf,
                                         uint8_t const* private_call_buf,
                                         uint8_t const* pk_buf,
                                         bool first_iteration,
                                         uint8_t const** proof_data_buf)
{
    // TODO accept proving key and use that to initialize composers
    // this info is just to prevent error for unused pk_buf
    // TODO do we want to accept it or just get it from our factory?
    (void)pk_buf; // unused
    auto crs_factory = std::make_shared<EnvReferenceStringFactory>();

    SignedTxRequest<NT> signed_tx_request;
    read(signed_tx_request_buf, signed_tx_request);

    PrivateCallData<NT> private_call_data;
    read(private_call_buf, private_call_data);

    PreviousKernelData<NT> previous_kernel;
    if (first_iteration) {
        previous_kernel = default_previous_kernel();
    } else {
        read(previous_kernel_buf, previous_kernel);
    }
    PrivateInputs<NT> private_inputs = PrivateInputs<NT>{
        .signed_tx_request = signed_tx_request,
        .previous_kernel = default_previous_kernel(),
        .private_call = private_call_data,
    };

    Composer private_kernel_composer = Composer(crs_factory);
    plonk::stdlib::types::Prover private_kernel_prover = private_kernel_composer.create_prover();

    PublicInputs<NT> public_inputs;
    public_inputs = private_kernel_circuit(private_kernel_composer, private_inputs);
    NT::Proof private_kernel_proof;
    private_kernel_proof = private_kernel_prover.construct_proof();

    // copy proof data to output buffer
    auto raw_proof_buf = (uint8_t*)malloc(private_kernel_proof.proof_data.size());
    memcpy(raw_proof_buf, (void*)private_kernel_proof.proof_data.data(), private_kernel_proof.proof_data.size());
    *proof_data_buf = raw_proof_buf;

    return private_kernel_proof.proof_data.size();
}

WASM_EXPORT size_t private_kernel__verify_proof(uint8_t const* vk_buf, uint8_t const* proof, uint32_t length)
{
    (void)vk_buf; // unused
    (void)proof;  // unused
    (void)length; // unused
    return true;
}

} // extern "C"