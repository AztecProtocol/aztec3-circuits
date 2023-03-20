#pragma once
#include <stdlib/types/circuit_types.hpp>
#include <stdlib/types/convert.hpp>
#include <stdlib/types/native_types.hpp>
#include <aztec3/constants.hpp>

namespace aztec3::circuits::abis {

using plonk::stdlib::types::CircuitTypes;
using plonk::stdlib::types::NativeTypes;
using std::is_same;

template <typename NCT> struct AppendOnlyTreeSnapshot {

    typedef typename NCT::fr fr;
    typedef typename NCT::uint32 uint32;

    fr root;
    uint32 next_available_leaf_index;

    bool operator==(AppendOnlyTreeSnapshot<NCT> const&) const = default;

    static AppendOnlyTreeSnapshot<NCT> empty() { return { 0, 0 }; };
};

template <typename NCT> void read(uint8_t const*& it, AppendOnlyTreeSnapshot<NCT>& obj)
{
    using serialize::read;

    read(it, obj.root);
    read(it, obj.next_available_leaf_index);
};

template <typename NCT> void write(std::vector<uint8_t>& buf, AppendOnlyTreeSnapshot<NCT> const& obj)
{
    using serialize::write;

    write(buf, obj.root);
    write(buf, obj.next_available_leaf_index);
};

template <typename NCT> std::ostream& operator<<(std::ostream& os, AppendOnlyTreeSnapshot<NCT> const& obj)
{
    return os << "root: " << obj.root << "\n"
              << "next_available_leaf_index: " << obj.next_available_leaf_index << "\n";
}

} // namespace aztec3::circuits::abis