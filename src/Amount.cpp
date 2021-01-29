/// \file Amount.cpp
/// \author jlippuner
/// \since Apr 21, 2018
///
/// \brief
///
///

#include "Amount.hpp"

template <uint D>
FixedPoint10<D>::FixedPoint10(int128_t i, int magnitude) {
  if (magnitude < -(int)D) {
    throw std::invalid_argument("Cannot create FixedPoint10 with magnitude " +
                                std::to_string(magnitude) +
                                " and number of digits " + std::to_string(D));
  }
  val_ = i * ipow_(10, magnitude + D);
}

template <uint D>
FixedPoint10<D> FixedPoint10<D>::Parse(const std::string& input_str) {
  // remove commas
  std::string str = "";
  for (auto c : input_str) {
    if (c != ',') str += c;
  }

  // handle exponents
  std::transform(str.begin(), str.end(), str.begin(),
      [](unsigned char c) { return std::tolower(c); });
  auto eloc = str.find('e');
  if (eloc != std::string::npos) {
    auto number = str.substr(0, eloc);
    int exp = std::stoi(str.substr(eloc + 1));

    if (exp == 0) {
      // nothing to do
      str = number;
    } else {
      // need to move the decimal place accordingly
      auto move_decimal = [](const std::string& s, bool forward) {
        auto dec = s.find('.');
        auto res = s;

        // first make sure we have a decimal point
        if (dec == std::string::npos) {
          res += ".0";
          dec = res.size() - 2;
        } else if (dec == 0) {
          res = "0" + s;
          dec = 1;
        } else if (dec == s.size() - 1) {
          res = s + "0";
          dec = res.size() - 2;
        }

        // swap decimal point with digit before/after it
        auto dig = dec + (forward ? -1 : 1);
        res[dec] = res[dig];
        res[dig] = '.';

        return res;
      };

      bool move_forward = (exp < 0);
      for (int i = 0; i < abs(exp); ++i)
        number = move_decimal(number, move_forward);

      str = number;

      if (str.find('.') == 0)
        str = "0" + str;
    }
  }

  std::regex reg(R"(^\s*(-?|\+?)([0-9]+)(?:\.([0-9]+)|\.)?\s*$)");
  std::smatch m;
  if (!std::regex_match(str, m, reg))
    throw std::invalid_argument("'" + str + "' is not a valid amount");

  // the match has 2 or 3 groups, first group is a negative sign or empty,
  // 2nd group is the integer part, 3rd group, if present, is the fractional
  // part
  if ((m.size() != 3) && (m.size() != 4))
    throw std::invalid_argument("'" + str + "' is not a valid amount");

  bool negative = (m[1].str() == "-");
  int128_t val(m[2].str());
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

    // now need to get rid of leading 0s
    while (frac[0] == '0') frac = frac.substr(1);

    val += int128_t(frac);
  }

  if (negative) val = -val;

  return FixedPoint10(val);
}

template <uint D>
std::string FixedPoint10<D>::ToStr() const {
  uint128_t uint_part =
      (uint128_t)boost::multiprecision::abs(val_) / Denominator();
  int128_t frac_part =
      boost::multiprecision::abs(val_) - (uint_part * Denominator());
  if (frac_part < 0)
    throw std::runtime_error("Something went wrong in FixedPoint10::ToStr");

  uint128_t ufrac = (uint128_t)frac_part;

  std::stringstream num;
  if (val_ < 0) num << "-";
  num << uint_part;
  num << ".";
  num << std::right << std::setfill('0') << std::setw(D) << ufrac;
  return num.str();
}

// explicit template instantiation
template class FixedPoint10<20>;
