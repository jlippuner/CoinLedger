#ifndef SRC_TAXES_BASIS_HPP_
#define SRC_TAXES_BASIS_HPP_

#include <deque>

#include "Acquisition.hpp"

class Basis {
 public:
  Basis(const std::vector<Acquisition>& acquisitions, bool LIFO);

  // Sell the given amount and return the consumed acquisitions
  std::vector<Acquisition> Sell(Amount amount);

  // get the total unsold amount and cost
  std::pair<Amount, Amount> Unsold() const;

 private:
  std::deque<Acquisition> basis_;
};

#endif  // SRC_TAXES_BASIS_HPP_