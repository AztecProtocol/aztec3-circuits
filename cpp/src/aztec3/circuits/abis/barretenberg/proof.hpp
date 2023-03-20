#pragma once
#include <common/serialize.hpp>
#include <stdlib/types/native_types.hpp>

namespace serialize {
using Proof = plonk::stdlib::types::NativeTypes::Proof;

inline void read(uint8_t const*& it, Proof& proof)
{
    using serialize::read;

    read(it, proof.proof_data);
};
} // namespace serialize

// TODO: Uncommenting this yields a duplicate symbol error in the linker which I haven't been able to solve
// namespace std {
// std::ostream& operator<<(std::ostream& os, plonk::stdlib::types::NativeTypes::Proof const& data)
// {
//     return os << data.proof_data;
// }
// } // namespace std