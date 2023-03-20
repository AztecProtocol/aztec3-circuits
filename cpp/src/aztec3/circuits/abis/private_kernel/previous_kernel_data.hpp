#pragma once
#include "proof_system/verification_key/verification_key.hpp"
#include "public_inputs.hpp"
#include "../barretenberg/proof.hpp"
#include <stdlib/primitives/witness/witness.hpp>
#include <stdlib/types/native_types.hpp>
#include <stdlib/types/circuit_types.hpp>
#include <stdlib/types/convert.hpp>

namespace aztec3::circuits::abis::private_kernel {

using plonk::stdlib::witness_t;
using plonk::stdlib::types::CircuitTypes;
using plonk::stdlib::types::NativeTypes;
using std::is_same;

template <typename NCT> struct PreviousKernelData {
    typedef typename NCT::fr fr;
    typedef typename NCT::VK VK;
    typedef typename NCT::uint32 uint32;

    PublicInputs<NCT> public_inputs; // TODO: not needed as already contained in proof?
    NativeTypes::Proof proof;        // TODO: how to express proof as native/circuit type when it gets used as a buffer?
    std::shared_ptr<VK> vk;

    // TODO: this index and path are meant to be those of a leaf within the tree of _kernel circuit_ vks; not the tree
    // of functions within the contract tree.
    uint32 vk_index;
    std::array<fr, VK_TREE_HEIGHT> vk_path;

    // WARNING: the `proof` does NOT get converted!
    template <typename Composer> PreviousKernelData<CircuitTypes<Composer>> to_circuit_type(Composer& composer) const
    {
        typedef CircuitTypes<Composer> CT;
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the composer:
        auto to_ct = [&](auto& e) { return plonk::stdlib::types::to_ct(composer, e); };

        PreviousKernelData<CircuitTypes<Composer>> data = {
            public_inputs.to_circuit_type(composer),
            proof, // Notice: not converted! Stays as native.
            CT::VK::from_witness(&composer, vk),
            to_ct(vk_index),
            to_ct(vk_path),
        };

        return data;
    };
}; // namespace aztec3::circuits::abis::private_kernel

// TODO(AD): After Milestone 1, rewrite this with better injection mechanism.
// Defined in c_bind.cpp.
std::shared_ptr<VerifierReferenceString> get_global_verifier_reference_string();

template <typename B> inline void read(B& buf, verification_key& key)
{
    using serialize::read;
    // Note this matches write() below
    verification_key_data data;
    read(buf, data);
    key = verification_key{ std::move(data), get_global_verifier_reference_string() };
}

template <typename NCT> void read(uint8_t const*& it, PreviousKernelData<NCT>& kernel_data)
{
    using serialize::read;

    read(it, kernel_data.public_inputs);
    read(it, kernel_data.proof);
    // Note: matches the structure of write verification_key
    verification_key_data vk_data;
    read(it, vk_data);
    kernel_data.vk = bonk::verification_key{ std::move(vk_data), get_global_verifier_reference_string() };
    // read(it, kernel_data.vk);
    read(it, kernel_data.vk_index);
    read(it, kernel_data.vk_path);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, PreviousKernelData<NCT> const& kernel_data)
{
    using serialize::write;

    write(buf, kernel_data.public_inputs);
    write(buf, kernel_data.proof);
    // Note: matches the structure of read verification_key_data
    write(buf, *kernel_data.vk.get());
    write(buf, kernel_data.vk_index);
    write(buf, kernel_data.vk_path);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, PreviousKernelData<NCT> const& kernel_data)
{
    return os << "public_inputs: " << kernel_data.public_inputs << "\n"
              << "proof: " << kernel_data.proof << "\n"
              << "vk: " << kernel_data.vk << "\n"
              << "vk_index: " << kernel_data.vk_index << "\n"
              << "vk_path: " << kernel_data.vk_path << "\n";
}

} // namespace aztec3::circuits::abis::private_kernel