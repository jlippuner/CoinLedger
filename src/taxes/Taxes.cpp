#include "Taxes.hpp"

#include <algorithm>
#include <list>

#include "PriceSource.hpp"

Taxes::Taxes(const File& file, Datetime until, Accnt assets,
    Accnt expense_mining_fees, Accnt expense_trading_fees,
    Accnt expense_transaction_fees, Accnt income_forks, Accnt income_mining) {
  // collect all mining income from the same day into one tax event
  std::unordered_map<std::string, std::map<Datetime, TaxEvent>> mining;

  // loop over all transactions and figure out what kind of tax event it is,
  // some transactions might be multiple tax events (e.g. trading one crypto
  // currency against another results in a tax event for both, or a trade
  // against USD with a fee results in a trade event and a spend event (the fee
  // is spent))
  for (auto itm : file.Transactions()) {
    auto txn = itm.second;
    if (txn->Date() > until) continue;

    // copy the splits of this transaction into a list, we will erase splits
    // from this list as we consume splits
    std::list<std::shared_ptr<const Split>> splits(
        txn->Splits().begin(), txn->Splits().end());

    // check if this is mining income
    {
      std::shared_ptr<const Split> mining_income_split = nullptr;
      for (auto it = splits.begin(); it != splits.end(); ++it) {
        if ((*it)->GetAccount()->IsContainedIn(income_mining)) {
          mining_income_split = *it;
          splits.erase(it);
          break;
        }
      }

      if (mining_income_split != nullptr) {
        // this is mining income, make sure this is a single-coin transaction
        auto coin = txn->GetCoin();
        if (coin == nullptr) {
          txn->Print(true);
          throw std::runtime_error("Got a multi-coin mining transaction");
        }

        // we expect to have an asset split and maybe a mining fee split
        std::shared_ptr<const Split> asset = nullptr;
        std::shared_ptr<const Split> fee = nullptr;

        auto it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->GetAccount()->IsContainedIn(assets)) {
            asset = *it;
            it = splits.erase(it);
            continue;
          }
          if ((*it)->GetAccount()->IsContainedIn(expense_mining_fees)) {
            fee = *it;
            it = splits.erase(it);
            continue;
          }
          ++it;
        }

        if (splits.size() > 0) {
          txn->Print(true);
          throw std::runtime_error("Leftover splits in mining transaction");
        }

        if (asset == nullptr) {
          txn->Print(true);
          throw std::runtime_error("No asset split in mining transaction");
        }

        Amount amt = asset->GetAmount();
        if (amt <= 0) {
          txn->Print(true);
          throw std::runtime_error("Expect positive mining income");
        }

        // we ignore the fee since that is not actually spent, we just acquire
        // the net mining income
        auto day = txn->Date().EndOfDay();
        if (mining[coin->Id()].count(day) == 0) {
          mining[coin->Id()].insert(
              {{day, TaxEvent(day, 0, 0, EventType::MiningIncome)}});
        }
        mining[coin->Id()].at(day).amount += amt;
        mining[coin->Id()].at(day).amount_usd +=
            amt * PriceSource::GetUSDPrice(day, coin);
      }
    }  // mining transaction
  }

  // add mining events
  for (auto& it : mining) {
    auto coin = it.first;
    for (auto& itm : it.second) {
      events_[coin].push_back(itm.second);
    }
  }

  // sort events by time
  for (auto& it : events_) {
    std::sort(it.second.begin(), it.second.end(),
        [](const TaxEvent& a, const TaxEvent& b) { return a.date < b.date; });
  }
}

void Taxes::PrintMiningIncome(const File& file) const {
  for (auto& it : events_) {
    auto coin = file.GetCoin(it.first);
    for (auto e : it.second) {
      if (e.type == EventType::MiningIncome) {
        printf("%s  %28s %4s = %28s USD\n", e.date.ToStrDayUTC().c_str(),
            e.amount.ToStr().c_str(), coin->Symbol().c_str(),
            e.amount_usd.ToStr().c_str());
      }
    }
  }
}
