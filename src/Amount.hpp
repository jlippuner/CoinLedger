/// \file Amount.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_AMOUNT_HPP_
#define SRC_AMOUNT_HPP_

#include <cmath>
#include <cstdint>
#include <regex>
#include <string>

typedef uint32_t uint;

namespace {

uint64_t ipow_(uint base, uint exp) {
  return exp > 1 ? ipow_(base, (exp >> 1) + (exp & 1)) * ipow_(base, exp >> 1)
                 : base;
}
}

// D is number of digits after the decimal point
template <uint D>
class FixedPoint10 {
  static_assert(D < 17, "FixedPoint10 can have at most 17 digits");

 public:
  static uint64_t Denominator() { return ipow_(10, D); }

  FixedPoint10() : val_(0) {}

  FixedPoint10(int i) : val_(i * Denominator()) {}

  FixedPoint10(double d) {
    double intpart;
    double frac = modf(d, &intpart);
    int64_t val = (int64_t)intpart * Denominator();
    int64_t int_frac = frac * (double)Denominator();
    val_ = val + int_frac;
  }

  static FixedPoint10 Parse(const std::string& str) {
    std::regex reg(R"(^\s*(-?)([0-9]+)(?:\.([0-9]+)|\.)?\s*$)");
    std::smatch m;
    if (!std::regex_match(str, m, reg))
      throw std::invalid_argument("'" + str + "' is not a valid amount");

    // the match has 2 or 3 groups, first group is a negative sign or empty,
    // 2nd group is the integer part, 3rd group, if present, is the fractional
    // part
    if ((m.size() != 3) && (m.size() != 4))
      throw std::invalid_argument("'" + str + "' is not a valid amount");

    bool negative = (m[1].str() == "-");
    int64_t val = std::stoull(m[2].str());
    val *= Denominator();

    if (m.size() == 4) {
      // we have a fractional part
      auto frac = m[3].str();

      if (frac.length() > D) {
        // there are too many decimal digits, truncate to the first D
        frac = frac.substr(0, D);
      } else if (frac.length() < D) {
        // there are too few decimal digits, add 0's up to length D
        while (frac.length() < D) frac += "0";
      }

      // we now have a string of exactly D digits
      if (frac.length() != D)
        throw std::runtime_error("Something went wrong in FixedPoint10::Parse");

      val += std::stoull(frac);
    }

    if (negative) val = -val;

    return FixedPoint10(val);
  }

  static FixedPoint10 FromRaw(int64_t val) { return FixedPoint10(val); }

  int64_t Raw() const { return val_; }

  std::string ToStr() const {
    uint64_t uint_part = labs(val_) / Denominator();
    int64_t frac_part = labs(val_) - (uint_part * Denominator());
    if (frac_part < 0)
      throw std::runtime_error("Something went wrong in FixedPoint10::ToStr");

    uint64_t ufrac = (uint64_t)frac_part;

    char format[16];
    char str[64];
    sprintf(format, "%%lu.%%0%ulu", D);
    sprintf(str, format, uint_part, ufrac);

    return ((val_ < 0) ? "-" : "") + std::string(str);
  }

  // comparison operators
  bool operator==(const FixedPoint10& other) const {
    return val_ == other.val_;
  }
  bool operator!=(const FixedPoint10& other) const {
    return val_ != other.val_;
  }
  bool operator>(const FixedPoint10& other) const { return val_ > other.val_; }
  bool operator<(const FixedPoint10& other) const { return val_ < other.val_; }
  bool operator>=(const FixedPoint10& other) const {
    return val_ >= other.val_;
  }
  bool operator<=(const FixedPoint10& other) const {
    return val_ <= other.val_;
  }

  // arithmetic operators
  FixedPoint10 operator-() const { return FixedPoint10(-val_); }
  FixedPoint10 operator+(const FixedPoint10& other) const {
    return FixedPoint10(val_ + other.val_);
  }
  FixedPoint10& operator+=(const FixedPoint10& other) {
    val_ += other.val_;
    return *this;
  }
  FixedPoint10 operator-(const FixedPoint10& other) const {
    return FixedPoint10(val_ - other.val_);
  }
  FixedPoint10& operator-=(const FixedPoint10& other) {
    val_ -= other.val_;
    return *this;
  }

 private:
  FixedPoint10(int64_t val) : val_(val) {}

  int64_t val_;
};

using Amount = FixedPoint10<10>;

#endif  // SRC_AMOUNT_HPP_
