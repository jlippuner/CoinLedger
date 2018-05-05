#ifndef SRC_TAXES_ACQUISITION_HPP_
#define SRC_TAXES_ACQUISITION_HPP_

#include "Amount.hpp"
#include "Datetime.hpp"

enum class SourceType { Mining, Trading, Fork, Other };

struct Acquisition {
  Acquisition(
      Datetime date, Amount amount, Amount cost_in_usd, SourceType source)
      : date(date), amount(amount), cost_in_usd(cost_in_usd), source(source) {}

  Datetime date;
  Amount amount;
  Amount cost_in_usd;
  SourceType source;
};

#endif  // SRC_TAXES_ACQUISITION_HPP_