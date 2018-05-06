#ifndef SRC_TAXES_TAXES_HPP_
#define SRC_TAXES_TAXES_HPP_

#include <map>

#include "Account.hpp"
#include "Amount.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"
#include "File.hpp"

enum class EventType {
  MiningIncome,
  ForkIncome,
  SpentGeneral,
  SpentTransactionFee,
  SpentTradingFee,
  TradeBuy,
  TradeSell
};

struct TaxEvent {
  TaxEvent(Datetime date, Amount amount, Amount amount_usd, EventType type)
      : date(date), amount(amount), amount_usd(amount_usd), type(type) {}

  TaxEvent(Datetime date, Amount amount, Amount amount_usd, EventType type,
      std::string memo)
      : date(date),
        amount(amount),
        amount_usd(amount_usd),
        type(type),
        memo(memo) {}

  Datetime date;
  Amount amount;
  Amount amount_usd;
  EventType type;
  std::string memo;
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

  Taxes(const File& file, Datetime until, Accnt assets, Accnt exchanges,
      Accnt equity, Accnt expenses, Accnt expense_mining_fees,
      Accnt expense_trading_fees, Accnt expense_transaction_fees,
      Accnt income_forks, Accnt income_mining);

  void PrintEvents(const File& file, EventType type) const;

  void PrintIncome(const File& file) const;
  void PrintSpending(const File& file) const;

 private:
  std::map<std::string, std::vector<TaxEvent>> events_;
};

#endif  // SRC_TAXES_TAXES_HPP_