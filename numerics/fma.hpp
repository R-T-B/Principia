#pragma once

#include <immintrin.h>

#include "base/cpuid.hpp"
#include "base/macros.hpp"  // ðŸ§™ For PRINCIPIA_USE_FMA_IF_AVAILABLE.

namespace principia {
namespace numerics {
namespace _fma {
namespace internal {

using namespace principia::base::_cpuid;

inline bool const UseHardwareFMA = true;

// âŸ¦ab + câŸ§.
inline double FusedMultiplyAdd(double a, double b, double c);

// âŸ¦ab - câŸ§.
inline double FusedMultiplySubtract(double a, double b, double c);

// âŸ¦-ab + câŸ§.
inline double FusedNegatedMultiplyAdd(double a, double b, double c);

// âŸ¦-ab - câŸ§.
inline double FusedNegatedMultiplySubtract(double a, double b, double c);

}  // namespace internal

using internal::FusedMultiplyAdd;
using internal::FusedMultiplySubtract;
using internal::FusedNegatedMultiplyAdd;
using internal::FusedNegatedMultiplySubtract;
using internal::UseHardwareFMA;

}  // namespace _fma
}  // namespace numerics
}  // namespace principia

#include "numerics/fma_body.hpp"
