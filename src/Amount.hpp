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

#include <boost/multiprecision/cpp_int.hpp>

#include <sqlite3.h>

typedef uint32_t uint;
typedef boost::multiprecision::checked_uint128_t uint128_t;
typedef boost::multiprecision::checked_int128_t int128_t;

namespace {

uint128_t ipow_(uint base, uint exp) {
  return exp > 1 ? ipow_(base, (exp >> 1) + (exp & 1)) * ipow_(base, exp >> 1)
                 : base;
}
}

// D is number of digits after the decimal point
template <uint D>
class FixedPoint10 {
  static_assert(D <= 20, "FixedPoint10 can have at most 20 digits");

 public:
  static uint128_t Denominator() { return ipow_(10, D); }

  FixedPoint10() : val_(0) {}

  FixedPoint10(int i) : val_(i * Denominator()) {}

  FixedPoint10(int128_t i, int magnitude);

  static FixedPoint10 Parse(const std::string& str);

  static FixedPoint10 FromRaw(const void* ptr) {
    int128_t val;
    memcpy(&val, ptr, size());
    return FixedPoint10(val);
  }

  static size_t size() { return sizeof(int128_t); }
  const void* Raw() const { return (void*)&val_; }

  std::string ToStr() const;

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
  FixedPoint10 operator*(const FixedPoint10& other) const {
    return FixedPoint10(val_ * other.val_);
  }

 private:
  FixedPoint10(int128_t val) : val_(val) {}

  int128_t val_;
};

using Amount = FixedPoint10<20>;

inline int sqlite3_bind_amount(
    sqlite3_stmt* stmt, int pos, const Amount& amount) {
  return sqlite3_bind_blob(
      stmt, pos, amount.Raw(), Amount::size(), SQLITE_TRANSIENT);
}

inline Amount sqlite3_column_amount(sqlite3_stmt* stmt, int iCol) {
  const void* ptr = sqlite3_column_blob(stmt, iCol);

  if (ptr == nullptr) {
    throw std::runtime_error("Database contains empty Amount");
  } else {
    if ((size_t)sqlite3_column_bytes(stmt, iCol) != Amount::size()) {
      throw std::runtime_error("Invalid Amount in database");
    }
    return Amount::FromRaw(ptr);
  }
}

#endif  // SRC_AMOUNT_HPP_
