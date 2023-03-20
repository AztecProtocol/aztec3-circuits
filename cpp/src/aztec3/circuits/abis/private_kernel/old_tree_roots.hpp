#pragma once
#include <stdlib/primitives/witness/witness.hpp>
#include <stdlib/types/native_types.hpp>
#include <stdlib/types/circuit_types.hpp>
#include <stdlib/types/convert.hpp>

namespace aztec3::circuits::abis::private_kernel {

using plonk::stdlib::witness_t;
using plonk::stdlib::types::CircuitTypes;
using plonk::stdlib::types::NativeTypes;
using std::is_same;

template <typename NCT> struct OldTreeRoots {
    typedef typename NCT::fr fr;

    fr private_data_tree_root;
    fr nullifier_tree_root;
    fr contract_tree_root;
    fr private_kernel_vk_tree_root; // TODO: future enhancement

    template <typename Composer> OldTreeRoots<CircuitTypes<Composer>> to_circuit_type(Composer& composer) const
    {
        static_assert((std::is_same<NativeTypes, NCT>::value));

        // Capture the composer:
        auto to_ct = [&](auto& e) { return plonk::stdlib::types::to_ct(composer, e); };

        OldTreeRoots<CircuitTypes<Composer>> data = {
            to_ct(private_data_tree_root),
            to_ct(nullifier_tree_root),
            to_ct(contract_tree_root),
            to_ct(private_kernel_vk_tree_root),
        };

        return data;
    };

    template <typename Composer> OldTreeRoots<NativeTypes> to_native_type() const
    {
        static_assert(std::is_same<CircuitTypes<Composer>, NCT>::value);
        auto to_nt = [&](auto& e) { return plonk::stdlib::types::to_nt<Composer>(e); };

        OldTreeRoots<NativeTypes> data = {
            to_nt(private_data_tree_root),
            to_nt(nullifier_tree_root),
            to_nt(contract_tree_root),
            to_nt(private_kernel_vk_tree_root),
        };

        return data;
    };

    void set_public()
    {
        static_assert(!(std::is_same<NativeTypes, NCT>::value));

        private_data_tree_root.set_public();
        nullifier_tree_root.set_public();
        contract_tree_root.set_public();
        private_kernel_vk_tree_root.set_public();
    }
};

template <typename NCT> void read(uint8_t const*& it, OldTreeRoots<NCT>& old_tree_roots)
{
    using serialize::read;

    read(it, old_tree_roots.private_data_tree_root);
    read(it, old_tree_roots.nullifier_tree_root);
    read(it, old_tree_roots.contract_tree_root);
    read(it, old_tree_roots.private_kernel_vk_tree_root);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, OldTreeRoots<NCT> const& old_tree_roots)
{
    using serialize::write;

    write(buf, old_tree_roots.private_data_tree_root);
    write(buf, old_tree_roots.nullifier_tree_root);
    write(buf, old_tree_roots.contract_tree_root);
    write(buf, old_tree_roots.private_kernel_vk_tree_root);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, OldTreeRoots<NCT> const& old_tree_roots)
{
    return os << "private_data_tree_root: " << old_tree_roots.private_data_tree_root << "\n"
              << "nullifier_tree_root: " << old_tree_roots.nullifier_tree_root << "\n"
              << "contract_tree_root: " << old_tree_roots.contract_tree_root << "\n"
              << "private_kernel_vk_tree_root: " << old_tree_roots.private_kernel_vk_tree_root << "\n";
}

} // namespace aztec3::circuits::abis::private_kernel