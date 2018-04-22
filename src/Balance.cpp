/// \file Balance.cpp
/// \author jlippuner
/// \since Apr 8, 2018
///
/// \brief
///
///

#include "Balance.hpp"

void Balance::Print(std::string indent, bool flip_sign,
    const std::unordered_map<std::string, Amount>* prices) const {
  if (amounts_.size() == 0) return;

  std::vector<std::shared_ptr<const Coin>> coins;
  for (auto& am : amounts_) coins.push_back(am.first);

  // sort coins by symbol
  std::sort(coins.begin(), coins.end(),
      [](std::shared_ptr<const Coin> a, std::shared_ptr<const Coin> b) {
        return a->Symbol() < b->Symbol();
      });

  Amount total_usd = 0;

  for (auto& c : coins) {
    auto amt = amounts_.at(c);
    if (amt == 0) continue;

    if (flip_sign) amt = -amt;

    printf(
        "%s%28s %4s", indent.c_str(), amt.ToStr().c_str(), c->Symbol().c_str());

    if (prices != nullptr) {
      Amount usd = amt * prices->at(c->Id());
      total_usd += usd;
      printf(" = %28s USD\n", usd.ToStr().c_str());
    } else {
      printf("\n");
    }
  }

  // xxxx YYYY = xxxxx... USD
  //       Total zzzz     USD
  if (prices != nullptr) {
    printf("%s%28sTotal   %28s USD\n", indent.c_str(), "",
        total_usd.ToStr().c_str());
  }
}