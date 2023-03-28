#pragma once

#include "call_context_reconciliation_data.hpp"
#include "../call_stack_item.hpp"

#include <barretenberg/common/map.hpp>
#include <barretenberg/stdlib/primitives/witness/witness.hpp>
#include <aztec3/utils/types/native_types.hpp>
#include <aztec3/utils/types/circuit_types.hpp>
#include <aztec3/utils/types/convert.hpp>

namespace aztec3::circuits::abis::private_kernel {

using aztec3::utils::types::CircuitTypes;
using aztec3::utils::types::NativeTypes;
using plonk::stdlib::witness_t;
using std::is_same;

template <typename NCT> struct PrivateCallData {
    typedef typename NCT::address address;
    typedef typename NCT::fr fr;
    typedef typename NCT::VK VK;

    CallStackItem<NCT, CallType::Private> call_stack_item;

    std::array<CallStackItem<NCT, CallType::Private>, PRIVATE_CALL_STACK_LENGTH> private_call_stack_preimages;

    // std::array<CallStackItem<NCT, CallType::Public>, PUBLIC_CALL_STACK_LENGTH> public_call_stack_preimages;

    NativeTypes::Proof proof; // TODO: how to express proof as native/circuit type when it gets used as a buffer?
    std::shared_ptr<VK> vk;

    fr function_leaf_index;
    std::array<fr, VK_TREE_HEIGHT> function_leaf_path;

    fr contract_tree_root;
    fr contract_leaf_index;
    std::array<fr, CONTRACT_TREE_HEIGHT> contract_leaf_path;

    fr portal_contract_address; // an ETH address

    // WARNING: the `proof` does NOT get converted! (because the current implementation of `verify_proof` takes a proof
    // of native bytes; any conversion to circuit types happens within the `verify_proof` function)
    template <typename Composer> PrivateCallData<CircuitTypes<Composer>> to_circuit_type(Composer& composer) const
    {
        typedef CircuitTypes<Composer> CT;
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the composer:
        auto to_ct = [&](auto& e) { return aztec3::utils::types::to_ct(composer, e); };
        auto to_circuit_type = [&](auto& e) { return e.to_circuit_type(composer); };

        PrivateCallData<CircuitTypes<Composer>> data = {
            call_stack_item.to_circuit_type(composer),

            map(private_call_stack_preimages, to_circuit_type),

            proof, // Notice: not converted! Stays as native. This is because of how the verify_proof function
                   // currently works.
            CT::VK::from_witness(&composer, vk),

            to_ct(function_leaf_index),
            to_ct(function_leaf_path),

            to_ct(contract_tree_root),
            to_ct(contract_leaf_index),
            to_ct(contract_leaf_path),

            to_ct(portal_contract_address),
        };

        return data;
    };
};

template <typename NCT> void read(uint8_t const*& it, PrivateCallData<NCT>& private_call_data)
{
    using serialize::read;

    PrivateCallData<NCT>& pcd = private_call_data;

    read(it, pcd.call_stack_item);
    read(it, pcd.private_call_stack_preimages);
    read(it, pcd.proof.proof_data);
    read(it, pcd.vk);
    read(it, pcd.function_leaf_index);
    read(it, pcd.function_leaf_path);
    read(it, pcd.contract_tree_root);
    read(it, pcd.contract_leaf_index);
    read(it, pcd.contract_leaf_path);
    read(it, pcd.portal_contract_address);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, PrivateCallData<NCT> const& private_call_data)
{
    using serialize::write;

    PrivateCallData<NCT> const& pcd = private_call_data;

    write(buf, pcd.call_stack_item);
    write(buf, pcd.private_call_stack_preimages);
    write(buf, pcd.proof.proof_data);
    write(buf, pcd.vk);
    write(buf, pcd.function_leaf_index);
    write(buf, pcd.function_leaf_path);
    write(buf, pcd.contract_tree_root);
    write(buf, pcd.contract_leaf_index);
    write(buf, pcd.contract_leaf_path);
    write(buf, pcd.portal_contract_address);
};

} // namespace aztec3::circuits::abis::private_kernel