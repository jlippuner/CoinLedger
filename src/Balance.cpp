/// \file Balance.cpp
/// \author jlippuner
/// \since Apr 8, 2018
///
/// \brief
///
///

#include "Balance.hpp"

void Balance::Print(std::string indent) const {
  if (amounts_.size() == 0) return;

  std::vector<std::shared_ptr<const Coin>> coins;
  for (auto& am : amounts_) coins.push_back(am.first);

  // sort coins by symbol
  std::sort(coins.begin(), coins.end(),
      [](std::shared_ptr<const Coin> a, std::shared_ptr<const Coin> b) {
        return a->Symbol() < b->Symbol();
      });

  for (auto& c : coins) {
    printf("%s%18s %s\n", indent.c_str(), amounts_.at(c).ToStr().c_str(),
        c->Symbol().c_str());
  }
}