#pragma once

#include <memory>
#include <srs/reference_string/reference_string.hpp>

namespace aztec3::circuits::abis {

// TODO(AD): After Milestone 1, rewrite this with better injection mechanism.
std::shared_ptr<bonk::VerifierReferenceString> get_global_verifier_reference_string();
// TODO(AD): After Milestone 1, rewrite this with better injection mechanism.
void set_global_verifier_reference_string(std::shared_ptr<bonk::VerifierReferenceString> const& vrs);

} // namespace aztec3::circuits::abis