#include "Inventory.hpp"
#include "Datetime.hpp"

void Inventory::Acquire(InventoryItem item) {
  if ((basis_.size() > 0) && (item.date < basis_.back().date))
    throw std::runtime_error("Going backwards in time in Inventory::Acquire");

  basis_.push_back(item);
}

std::vector<InventoryItem> Inventory::Dispose(Amount amount) {
  std::vector<InventoryItem> consumed;
  Amount remaining = amount;

  while (remaining > 0) {
    if (basis_.size() == 0)
      throw std::runtime_error("Amount remaining but no inventory");

    auto& match = LIFO_ ? basis_.back() : basis_.front();

    if (remaining >= match.amount) {
      // completely consume the match
      consumed.push_back(match);
      remaining -= match.amount;

      if (LIFO_)
        basis_.pop_back();
      else
        basis_.pop_front();
    } else {
      // partially consume the latest acquisition
      InventoryItem partial(match);

      partial.amount = remaining;
      partial.cost_in_usd = (match.cost_in_usd * remaining) / match.amount;

      match.amount -= partial.amount;
      match.cost_in_usd -= partial.cost_in_usd;

      consumed.push_back(partial);
      remaining = 0;
    }
  }

  return consumed;
}

UnsoldInventory Inventory::Unsold(size_t long_term_in_days) const {
  UnsoldInventory res;
  auto now = Datetime::Now();

  for (auto& a : basis_) {
    bool long_term = (a.date.AbsDiffInSeconds(now) > (long_term_in_days * 24 * 3600));

    (long_term ? res.long_term : res.short_term).amount += a.amount;
    (long_term ? res.long_term : res.short_term).cost_in_usd += a.cost_in_usd;

    res.total.amount += a.amount;
    res.total.cost_in_usd += a.cost_in_usd;
  }

  return res;
}