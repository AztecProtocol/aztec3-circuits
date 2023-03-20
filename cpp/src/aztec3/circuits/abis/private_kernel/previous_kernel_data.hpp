#pragma once
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

template <typename NCT> void read(uint8_t const*& it, PreviousKernelData<NCT>& kernel_data)
{
    using serialize::read;

    read(it, kernel_data.public_inputs);
    read(it, kernel_data.proof);
    // read(it, kernel_data.vk); // TODO: Serialize VK
    read(it, kernel_data.vk_index);
    read(it, kernel_data.vk_path);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, PreviousKernelData<NCT> const& kernel_data)
{
    using serialize::write;

    write(buf, kernel_data.public_inputs);
    write(buf, kernel_data.proof);
    // write(buf, kernel_data.vk); // TODO: Serialize VK
    write(buf, kernel_data.vk_index);
    write(buf, kernel_data.vk_path);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, PreviousKernelData<NCT> const& kernel_data)
{
    return os << "public_inputs: " << kernel_data.public_inputs
              << "\n"
              // TODO: Use << operator from cpp/src/aztec3/circuits/abis/barretenberg/proof.hpp instead of inlining
              << "proof: " << kernel_data.proof.proof_data << "\n"
              << "vk: " << kernel_data.vk << "\n"
              << "vk_index: " << kernel_data.vk_index << "\n"
              << "vk_path: " << kernel_data.vk_path << "\n";
}

} // namespace aztec3::circuits::abis::private_kernel