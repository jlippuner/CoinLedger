#ifndef SRC_TAXES_INVENTORY_HPP_
#define SRC_TAXES_INVENTORY_HPP_

#include <deque>

#include "Amount.hpp"
#include "Datetime.hpp"

struct InventoryItem {
  InventoryItem(Datetime date, Amount amount, Amount cost_in_usd)
      : date(date), amount(amount), cost_in_usd(cost_in_usd) {}

  Datetime date;
  Amount amount;
  Amount cost_in_usd;
};

struct UnsoldInventory {
  struct Unsold {
    Amount amount, cost_in_usd;
  };

  Unsold short_term, long_term, total;
};

class Inventory {
 public:
  Inventory(bool LIFO) : LIFO_(LIFO) {}

  void Acquire(InventoryItem item);

  // dispose the given amount and return the consumed inventory items
  std::vector<InventoryItem> Dispose(Amount amount);

  // get the total unsold amount and cost
  UnsoldInventory Unsold(size_t long_term_in_days) const;

 private:
  // true if this is a LIFO inventory, false if it's FIFO
  bool LIFO_;

  std::deque<InventoryItem> basis_;
};

#endif  // SRC_TAXES_INVENTORY_HPP_