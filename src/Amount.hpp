/// \file Amount.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_AMOUNT_HPP_
#define SRC_AMOUNT_HPP_

namespace {

constexpr uint64_t ipow_(uint base, uint exp) {
  return exp > 1 ? ipow_(base, (exp>>1) + (exp&1)) * ipow_(base, exp>>1) : base;
}

}

// D is number of digits after the decimal point
template<uint D>
class FixedPoint10 {
  static_assert(D < 17, "FixedPoint10 can have at most 17 digits");
public:
  constexpr uint64_t Denominator = ipow_(10, D);

private:
  int64_t val_;
};

using Amount = FixedPoint10<10>;

#endif // SRC_AMOUNT_HPP_
