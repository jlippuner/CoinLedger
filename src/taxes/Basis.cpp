#include "Basis.hpp"

#include <algorithm>
#include <map>

Basis::Basis(const std::vector<Acquisition>& acquisitions, bool LIFO) {
  // collect all mining acquisitions from the same day into one acquisition
  std::map<Datetime, Acquisition> mining;

  std::vector<Acquisition> all_acquisitions;

  for (auto& a : acquisitions) {
    if (a.source == SourceType::Mining) {
      auto day = a.date.Day();
      if (mining.count(day) == 0) {
        mining.insert({{day, Acquisition(day, 0, 0, SourceType::Mining)}});
      }
      mining.at(day).amount += a.amount;
      mining.at(day).cost_in_usd += a.cost_in_usd;
    } else {
      // copy non-mining acquisitions
      all_acquisitions.push_back(a);
    }
  }

  // add mining acquisitions
  for (auto& itm : mining) all_acquisitions.push_back(itm.second);

  // sort all acquisition by date
  std::sort(all_acquisitions.begin(), all_acquisitions.end(),
      [=](const Acquisition& a, const Acquisition& b) {
        return a.date < b.date;
      });

  // if we are doing FIFO, reverse the order so that the earliest acquisition
  // will be sold first
  if (!LIFO) std::reverse(all_acquisitions.begin(), all_acquisitions.end());

  // now create the basis deque (which we really just use a as a stack)
  basis_ =
      std::deque<Acquisition>(all_acquisitions.begin(), all_acquisitions.end());
}

std::vector<Acquisition> Basis::Sell(Amount amount) {
  std::vector<Acquisition> sold;
  Amount remaining = amount;

  while (remaining > 0) {
    auto& last = basis_.back();

    if (remaining >= last.amount) {
      // completely consume the latest acquisition
      sold.push_back(last);
      remaining -= last.amount;
      basis_.pop_back();
    } else {
      // partially consume the latest acquisition
      auto partial = basis_.back();

      partial.amount = remaining;
      partial.cost_in_usd = (last.cost_in_usd * remaining) / last.amount;

      last.amount -= partial.amount;
      last.cost_in_usd -= partial.cost_in_usd;

      sold.push_back(partial);
      remaining = 0;
    }
  }

  return sold;
}

std::pair<Amount, Amount> Basis::Unsold() const {
  std::pair<Amount, Amount> res{0, 0};

  for (auto& a : basis_) {
    res.first += a.amount;
    res.second += a.cost_in_usd;
  }

  return res;
}