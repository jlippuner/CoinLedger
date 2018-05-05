#ifndef SRC_TAXES_TAXES_HPP_
#define SRC_TAXES_TAXES_HPP_

#include <unordered_map>

#include "Account.hpp"
#include "Amount.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"
#include "File.hpp"

enum class EventType { ForkIncome, MiningIncome, Trade, Spent };

struct TaxEvent {
  TaxEvent(Datetime date, Amount amount, Amount amount_usd, EventType type)
      : date(date), amount(amount), amount_usd(amount_usd), type(type) {}

  Datetime date;
  Amount amount;
  Amount amount_usd;
  EventType type;
};

struct GainLoss {
  GainLoss(std::shared_ptr<const Coin> coin, Amount amount, Datetime acquired,
      Datetime disposed, Amount proceeds, Amount cost)
      : coin(coin),
        amount(amount),
        acquired(acquired),
        disposed(disposed),
        proceeds(proceeds),
        cost(cost) {}

  std::shared_ptr<const Coin> coin;
  Amount amount;
  Datetime acquired, disposed;
  Amount proceeds, cost;
};

class Taxes {
 public:
  using Accnt = std::shared_ptr<const Account>;

  Taxes(const File& file, Datetime until, Accnt assets,
      Accnt expense_mining_fees, Accnt expense_trading_fees,
      Accnt expense_transaction_fees, Accnt income_forks, Accnt income_mining);

  void PrintMiningIncome(const File& file) const;

 private:
  std::unordered_map<std::string, std::vector<TaxEvent>> events_;
};

#endif  // SRC_TAXES_TAXES_HPP_