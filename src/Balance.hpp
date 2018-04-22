/// \file Balance.hpp
/// \author jlippuner
/// \since Apr 8, 2018
///
/// \brief
///
///

#ifndef SRC_BALANCE_HPP_
#define SRC_BALANCE_HPP_

#include <memory>
#include <unordered_map>

#include "Amount.hpp"
#include "Coin.hpp"
#include "Split.hpp"

class Balance {
 public:
  Balance() {}

  const std::unordered_map<std::shared_ptr<const Coin>, Amount>& Amounts()
      const {
    return amounts_;
  }

  void AddAmount(Amount amount, std::shared_ptr<const Coin> coin) {
    amounts_[coin] += amount;
  }

  void AddSplit(std::shared_ptr<Split> split) {
    AddAmount(split->GetAmount(), split->GetCoin());
  }

  void Print(std::string indent, bool flip_sign,
      const std::unordered_map<std::string, Amount>* prices) const;

  // arithmetic operators
  Balance& operator+=(const Balance& other) {
    for (auto& am : other.amounts_) {
      amounts_[am.first] += am.second;
    }
    return *this;
  }

 private:
  std::unordered_map<std::shared_ptr<const Coin>, Amount> amounts_;
};

#endif  // SRC_BALANCE_HPP_