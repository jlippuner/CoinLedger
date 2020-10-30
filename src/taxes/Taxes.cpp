#include "Taxes.hpp"

#include <algorithm>
#include <list>

#include "Inventory.hpp"

#include "prices/PriceSource.hpp"

Taxes::Taxes(const File& file, Datetime until, Accnt assets, Accnt wallets, Accnt ecr20_account,
    Accnt exchanges, Accnt equity, Accnt expenses, Accnt expense_mining_fees,
    Accnt expense_trading_fees, Accnt expense_transaction_fees,
    Accnt income_forks, Accnt income_airdrop, Accnt income_mining) {
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

    // copy the splits of this transaction into a list and combine splits of the
    // same coin in the same account, we will erase splits from this list as we
    // consume splits
    std::list<std::shared_ptr<ProtoSplit>> splits;

    for (auto& in_sp : txn->Splits()) {
      // check if a split with the same account and coin already exists
      bool already_exists = false;
      for (auto& sp : splits) {
        if ((in_sp->GetAccount()->Id() == sp->account_->Id()) &&
            (in_sp->GetCoin()->Id() == sp->coin_->Id())) {
          sp->amount_ += in_sp->GetAmount();
          already_exists = true;
          break;
        }
      }

      if (!already_exists) {
        splits.push_back(
            std::shared_ptr<ProtoSplit>(new ProtoSplit(in_sp->GetAccount(), "",
                in_sp->GetAmount(), in_sp->GetCoin(), "")));
      }
    }

    auto coin = txn->GetCoin();

    // check if this is mining income
    {
      std::shared_ptr<const ProtoSplit> mining_income_split = nullptr;
      for (auto it = splits.begin(); it != splits.end(); ++it) {
        if ((*it)->account_->IsContainedIn(income_mining)) {
          mining_income_split = *it;
          splits.erase(it);
          break;
        }
      }

      if (mining_income_split != nullptr) {
        // this is mining income, make sure this is a single-coin transaction
        if (coin == nullptr) {
          txn->Print(true);
          throw std::runtime_error("Got a multi-coin mining transaction");
        }

        // we expect to have an asset split and maybe a mining fee split
        std::shared_ptr<const ProtoSplit> asset = nullptr;
        std::shared_ptr<const ProtoSplit> fee = nullptr;

        auto it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->account_->IsContainedIn(assets)) {
            asset = *it;
            it = splits.erase(it);
            continue;
          }
          if ((*it)->account_->IsContainedIn(expense_mining_fees)) {
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

        Amount amt = asset->amount_;
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
            amt * file.GetHistoricUSDPrice(day, coin);

        // done with this transaction
        continue;
      }
    }  // mining transaction

    // check if this is fork income
    {
      std::shared_ptr<const ProtoSplit> fork_income_split = nullptr;
      for (auto it = splits.begin(); it != splits.end(); ++it) {
        if ((*it)->account_->IsContainedIn(income_forks) ||
            (*it)->account_->IsContainedIn(income_airdrop)) {
          fork_income_split = *it;
          splits.erase(it);
          break;
        }
      }

      if (fork_income_split != nullptr) {
        // this is fork income, make sure this is a single-coin transaction
        if (coin == nullptr) {
          txn->Print(true);
          throw std::runtime_error("Got a multi-coin fork income transaction");
        }

        // we expect to have an asset split and maybe a transaction fee split
        std::shared_ptr<const ProtoSplit> asset = nullptr;
        std::shared_ptr<const ProtoSplit> fee = nullptr;

        auto it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->account_->IsContainedIn(assets)) {
            asset = *it;
            it = splits.erase(it);
            continue;
          }
          if ((*it)->account_->IsContainedIn(expense_transaction_fees)) {
            fee = *it;
            it = splits.erase(it);
            continue;
          }
          ++it;
        }

        if (splits.size() > 0) {
          txn->Print(true);
          throw std::runtime_error(
              "Leftover splits in fork income transaction");
        }

        if (asset == nullptr) {
          txn->Print(true);
          throw std::runtime_error("No asset split in fork income transaction");
        }

        Amount amt = asset->amount_;
        if (amt <= 0) {
          txn->Print(true);
          throw std::runtime_error("Expect positive fork income");
        }

        // we ignore the fee since that is not actually spent, we just acquire
        // the net fork income at its USD value at the time of the fork
        Amount amt_usd = amt * file.GetHistoricUSDPrice(txn->Date(), coin);
        events_[coin->Id()].push_back(
            TaxEvent(txn->Date(), amt, amt_usd, EventType::ForkIncome));

        // done with this transaction
        continue;
      }
    }  // fork income

    // if this is a single coin transaction, record what is spent
    // also if this is a transfer transaction involving an ECR20 token with the
    // fee paid in ETH, record the fee
    {
      bool decrease_ECR20 = false;
      bool spend_ETH_txn_fee = false;
      for (auto &s : splits) {
        if (s->account_->IsContainedIn(ecr20_account) && (s->amount_ < 0))
          decrease_ECR20 = true;
        if (s->account_->IsContainedIn(expense_transaction_fees) &&
            (s->amount_ > 0) && (s->coin_->Symbol() == "ETH")) {
          spend_ETH_txn_fee = true;
          coin = s->coin_;
        }
      }

      if ((coin != nullptr) || (decrease_ECR20 && spend_ETH_txn_fee)) {
        std::vector<std::shared_ptr<const ProtoSplit>> expense_splits;

        auto it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->account_->IsContainedIn(assets) ||
              (*it)->account_->IsContainedIn(equity)) {
            it = splits.erase(it);
            continue;
          }
          if ((*it)->account_->IsContainedIn(expenses)) {
            expense_splits.push_back(*it);
            it = splits.erase(it);
            continue;
          }
          ++it;
        }

        if (splits.size() > 0) {
          txn->Print(true);
          throw std::runtime_error(
              "Leftover splits in single-coin transaction");
        }

        // make spend events for all expenses, but make sure no expenses are
        // trading fees or mining fees
        for (auto& e : expense_splits) {
          if (e->account_->IsContainedIn(expense_mining_fees)) {
            txn->Print(true);
            throw std::runtime_error("Unexpected mining fee expense");
          }
          if (e->account_->IsContainedIn(expense_trading_fees)) {
            txn->Print(true);
            throw std::runtime_error("Unexpected trading fee expense");
          }

          auto date = txn->Date();
          auto amt = e->amount_;
          auto usd = amt * file.GetHistoricUSDPrice(date, coin);
          EventType type = e->account_->IsContainedIn(expense_transaction_fees)
                               ? EventType::SpentTransactionFee
                               : EventType::SpentGeneral;
          std::string memo =
              txn->Description() + " (" + e->account_->FullName() + ")";
          events_[coin->Id()].push_back(TaxEvent(date, amt, usd, type, memo));
        }

        // done with this transaction
        continue;
      }
    }  // spending

    // check if this is spending with a coin conversion
    {
      // There is one split that reduces a wallet account, one that increases an
      // expense account (which is not a transaction, mining, or trading fee),
      // and optionally a transaction fee split

      // first find wallet split
      std::shared_ptr<const ProtoSplit> wallet_split = nullptr;
      auto it = splits.begin();
      while (it != splits.end()) {
        if ((*it)->account_->IsContainedIn(wallets)) {
          wallet_split = *it;
          it = splits.erase(it);
          break;
        }
        ++it;
      }

      if (wallet_split != nullptr) {
        if (wallet_split->amount_ >= 0) {
          txn->Print(true);
          throw std::runtime_error(
              "Expect a negative amount in the wallet split");
        }

        std::shared_ptr<const ProtoSplit> fee_split = nullptr;
        auto it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->account_->IsContainedIn(expense_transaction_fees)) {
            fee_split = *it;
            it = splits.erase(it);
            break;
          }
          ++it;
        }
        if ((fee_split != nullptr) && (fee_split->amount_ <= 0)) {
          txn->Print(true);
          throw std::runtime_error("Expect a positive transaction fee");
        }

        // find fee match split if there is one
        std::shared_ptr<const ProtoSplit> fee_match_split = nullptr;
        if (fee_split != nullptr) {
          it = splits.begin();
          while (it != splits.end()) {
            if (((*it)->coin_->Id() == fee_split->coin_->Id()) &&
                ((*it)->amount_ == -fee_split->amount_)) {
              fee_match_split = *it;
              it = splits.erase(it);
              continue;
            }
            ++it;
          }
        }

        // there should be one split left now, which is an expense split, but
        // not transaction, trading, or mining fee
        if (splits.size() != 1) {
          txn->Print(true);
          throw std::runtime_error(
              "Expected 1 split left in conversion spending");
        }

        // check the expense split
        std::shared_ptr<const ProtoSplit> expense_split = nullptr;
        it = splits.begin();
        while (it != splits.end()) {
          if ((*it)->account_->IsContainedIn(expenses) &&
              !(*it)->account_->IsContainedIn(expense_transaction_fees) &&
              !(*it)->account_->IsContainedIn(expense_mining_fees) &&
              !(*it)->account_->IsContainedIn(expense_trading_fees) &&
              ((*it)->amount_ > 0)) {
            expense_split = *it;
            it = splits.erase(it);
            break;
          }
          ++it;
        }

        if ((expense_split == nullptr) || (wallet_split == nullptr) ||
            (splits.size() > 0)) {
          txn->Print(true);
          throw std::runtime_error("Couldn't get buy and sell splits");
        }

        auto date = txn->Date();
        Amount fee_usd = 0;

        // get the USD amount from the sell split by default
        Amount amt_usd = -wallet_split->amount_ *
                         file.GetHistoricUSDPrice(date, wallet_split->coin_);

        // unless the buy coin is USD or USDT
        if (expense_split->coin_->IsUSD())
          amt_usd = expense_split->amount_;
        else if (expense_split->coin_->Id() == "tether")
          amt_usd = expense_split->amount_ *
                    file.GetHistoricUSDPrice(date, expense_split->coin_);

        if (fee_split != nullptr) {
          if ((fee_split->coin_->Id() == expense_split->coin_->Id()) ||
              (fee_split->coin_->Id() == wallet_split->coin_->Id())) {
            // make sure we don't have a fee match, don't need to treat the fee
            // separately, since it's already accounted for in the buy or sell
            // split
            if (fee_match_split != nullptr) {
              txn->Print(true);
              throw std::runtime_error("Unexpected fee match split");
            }
          } else {
            // make sure we have a fee match
            if (fee_match_split == nullptr) {
              txn->Print(true);
              throw std::runtime_error("Expected fee match split");
            }
            // the fee is not accounted for in the buy or sell split, determine
            // its USD value and spend the fee coin
            fee_usd = fee_split->amount_ *
                      file.GetHistoricUSDPrice(date, fee_split->coin_);
            std::string memo = txn->Description() + " (" +
                               fee_split->account_->FullName() + ")";
            events_[fee_split->coin_->Id()].push_back(
                TaxEvent(date, fee_split->amount_, fee_usd,
                    EventType::SpentTransactionFee, memo));
          }
        }

        // if the fee was paid in a 3rd coin (fee_usd > 0), account for the fee
        // in the basis (i.e. cost) of the coin that was bought
        std::string memo = txn->Description() + " (" +
                           expense_split->account_->FullName() + ")";
        events_[expense_split->coin_->Id()].push_back(
            TaxEvent(date, expense_split->amount_, amt_usd + fee_usd,
                EventType::SpentGeneral, memo));
        events_[wallet_split->coin_->Id()].push_back(TaxEvent(
            date, -wallet_split->amount_, amt_usd, EventType::TradeSell));

        // done with this transaction
        continue;
      }
    }  // conversion spending transaction

    // this has to be a trade transaction on an exchange
    {
      // All splits must either be under exchanges or trading fees, there is at
      // most one trading fee split.
      // There is one coin that is sold and one that is bought, and optionally,
      // a different coin in which the fee is paid.

      // first find fee
      std::shared_ptr<const ProtoSplit> fee_split = nullptr;
      auto it = splits.begin();
      while (it != splits.end()) {
        if ((*it)->account_->IsContainedIn(expense_trading_fees)) {
          fee_split = *it;
          it = splits.erase(it);
          break;
        }
        ++it;
      }

      if ((fee_split != nullptr) && (fee_split->amount_ <= 0)) {
        txn->Print(true);
        throw std::runtime_error("Expect a positive trading fee");
      }

      // find fee match split and make sure all splits are in the exchange
      // account
      std::shared_ptr<const ProtoSplit> fee_match_split = nullptr;
      it = splits.begin();
      while (it != splits.end()) {
        if (!(*it)->account_->IsContainedIn(exchanges)) {
          txn->Print(true);
          throw std::runtime_error(
              "Expected exchange split in trade transaction");
        }
        if (fee_split != nullptr) {
          if (((*it)->coin_->Id() == fee_split->coin_->Id()) &&
              ((*it)->amount_ == -fee_split->amount_)) {
            fee_match_split = *it;
            it = splits.erase(it);
            continue;
          }
        }
        ++it;
      }

      if (splits.size() != 2) {
        txn->Print(true);
        throw std::runtime_error("Expected 2 splits for a trade transaction");
      }

      // find buy and sell splits
      std::shared_ptr<const ProtoSplit> buy_split = nullptr;
      std::shared_ptr<const ProtoSplit> sell_split = nullptr;
      it = splits.begin();
      while (it != splits.end()) {
        if ((*it)->amount_ < 0) {
          sell_split = *it;
          it = splits.erase(it);
          continue;
        }
        if ((*it)->amount_ > 0) {
          buy_split = *it;
          it = splits.erase(it);
          continue;
        }
        ++it;
      }

      if ((buy_split == nullptr) || (sell_split == nullptr) ||
          (splits.size() > 0)) {
        txn->Print(true);
        throw std::runtime_error("Couldn't get buy and sell splits");
      }

      auto date = txn->Date();
      Amount fee_usd = 0;

      // get the USD amount from the sell split by default
      Amount amt_usd = -sell_split->amount_ *
                       file.GetHistoricUSDPrice(date, sell_split->coin_);

      // unless the buy coin is USD or USDT
      if (buy_split->coin_->IsUSD())
        amt_usd = buy_split->amount_;
      else if (buy_split->coin_->Id() == "tether")
        amt_usd = buy_split->amount_ *
                  file.GetHistoricUSDPrice(date, buy_split->coin_);

      if (fee_split != nullptr) {
        if ((fee_split->coin_->Id() == buy_split->coin_->Id()) ||
            (fee_split->coin_->Id() == sell_split->coin_->Id())) {
          // make sure we don't have a fee match, don't need to treat the fee
          // separately, since it's already accounted for in the buy or sell
          // split
          if (fee_match_split != nullptr) {
            txn->Print(true);
            throw std::runtime_error("Unexpected fee match split");
          }
        } else {
          // make sure we have a fee match
          if (fee_match_split == nullptr) {
            txn->Print(true);
            throw std::runtime_error("Expected fee match split");
          }
          // the fee is not accounted for in the buy or sell split, determine
          // its USD value and spend the fee coin
          fee_usd = fee_split->amount_ *
                    file.GetHistoricUSDPrice(date, fee_split->coin_);
          events_[fee_split->coin_->Id()].push_back(TaxEvent(
              date, fee_split->amount_, fee_usd, EventType::SpentTradingFee));
        }
      }

      // if the fee was paid in a 3rd coin (fee_usd > 0), account for the fee in
      // the basis (i.e. cost) of the coin that was bought
      events_[buy_split->coin_->Id()].push_back(TaxEvent(
          date, buy_split->amount_, amt_usd + fee_usd, EventType::TradeBuy));
      events_[sell_split->coin_->Id()].push_back(
          TaxEvent(date, -sell_split->amount_, amt_usd, EventType::TradeSell));

      // done with this transaction
      continue;
    }  // trade transaction

    // if we end up here, we couldn't handle this transaction
    txn->Print(true);
    throw std::runtime_error("Don't know how to handle that transaction");
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

void Taxes::PrintEvents(const File& file, EventType type, Datetime from) const {
  // first create a list of events to be printed and sort them by date
  using CoinEvent = std::pair<std::string, TaxEvent>;
  std::vector<CoinEvent> sorted_events;

  for (auto& it : events_) {
    for (auto e : it.second) {
      if ((e.type == type) && (e.date >= from)) {
        sorted_events.push_back({it.first, e});
      }
    }
  }

  std::sort(sorted_events.begin(), sorted_events.end(),
      [](const CoinEvent& a, const CoinEvent& b) {
        return a.second.date < b.second.date;
      });

  for (auto& ev : sorted_events) {
    auto coin = file.GetCoin(ev.first);
    auto& e = ev.second;
    printf("%s  %28s %4s = %28s USD  %s\n", e.date.ToStrDayUTC().c_str(),
        e.amount.ToStr().c_str(), coin->Symbol().c_str(),
        e.amount_usd.ToStr().c_str(), e.memo.c_str());
  }
}

void Taxes::PrintIncome(const File& file, Datetime from) const {
  printf("Fork income\n");
  printf("===========\n");
  PrintEvents(file, EventType::ForkIncome, from);
  printf("\n\n");

  printf("Mining income\n");
  printf("========================\n");
  PrintEvents(file, EventType::MiningIncome, from);
  printf("\n\n");
}

void Taxes::PrintSpending(const File& file, Datetime from) const {
  printf("General spending\n");
  printf("================\n");
  PrintEvents(file, EventType::SpentGeneral, from);
  printf("\n\n");

  printf("Transaction fee spending\n");
  printf("========================\n");
  PrintEvents(file, EventType::SpentTransactionFee, from);
  printf("\n\n");

  printf("Trading fee spending\n");
  printf("====================\n");
  PrintEvents(file, EventType::SpentTradingFee, from);
  printf("\n\n");
}

void Taxes::PrintCapitalGainsLosses(const File& file, size_t long_term_in_days,
    bool LIFO, Datetime from, bool fuse) const {
  std::vector<GainLoss> short_term;
  std::vector<GainLoss> long_term;

  std::map<std::string, Inventory> inventories;

  for (auto& it : events_) {
    auto coin_id = it.first;
    if (coin_id == Coin::USD_id()) {
      for (auto& e : it.second) {
        if (e.amount != e.amount_usd)
          throw std::runtime_error("Got USD event with mismatching amounts");
      }
      continue;
    }

    inventories.insert({{coin_id, Inventory(LIFO)}});

    for (auto& e : it.second) {
      if ((e.type == EventType::MiningIncome) ||
          (e.type == EventType::ForkIncome) ||
          (e.type == EventType::TradeBuy)) {
        InventoryItem new_inv(e.date, e.amount, e.amount_usd);
        inventories.at(coin_id).Acquire(new_inv);
      } else if ((e.type == EventType::SpentGeneral) ||
                 (e.type == EventType::SpentTransactionFee) ||
                 (e.type == EventType::SpentTradingFee) ||
                 (e.type == EventType::TradeSell)) {
        auto disp = inventories.at(coin_id).Dispose(e.amount);
        std::vector<GainLoss> gains;

        if (disp.size() == 0) {
          throw std::runtime_error("Got 0 disposals");
        } else if (disp.size() == 1) {
          // only one inventory item was consumed
          auto d = disp[0];
          if (d.amount != e.amount) throw std::runtime_error("Amount mismatch");
          gains.push_back(GainLoss(file.GetCoin(coin_id), e.amount, d.date,
              e.date, e.amount_usd, d.cost_in_usd));
        } else {
          for (auto& d : disp) {
            Amount this_proceed = (e.amount_usd * d.amount) / e.amount;
            gains.push_back(GainLoss(file.GetCoin(coin_id), d.amount, d.date,
                e.date, this_proceed, d.cost_in_usd));
          }
        }

        for (auto& g : gains) {
          size_t holding_period_in_seconds =
              g.acquired.AbsDiffInSeconds(g.disposed);
          if (holding_period_in_seconds > (long_term_in_days * 24 * 3600))
            long_term.push_back(g);
          else
            short_term.push_back(g);
        }
      }
    }
  }

  printf("Short-Term Disposition of Assets\n");
  printf("================================\n");
  PrintGainLoss(&short_term, fuse, from);
  printf("\n\n");

  printf("Long-Term Disposition of Assets\n");
  printf("===============================\n");
  PrintGainLoss(&long_term, fuse, from);
  printf("\n\n");

  printf("Unrealized Gains and Losses\n");
  printf("===========================\n");
  printf("%34s  %28s  %28s  %28s\n", "Unsold Asset", "Net Cost (USD)",
      "Current Value (USD)", "Unrealized Profit/Loss (USD)");

  auto prices = PriceSource::GetUSDPrices();
  for (auto& it : inventories) {
    auto coin = file.GetCoin(it.first);

    auto unsold = it.second.Unsold();
    auto amount = unsold.first;
    if (amount == 0) continue;
    auto cost = unsold.second;
    auto value = amount * prices.at(coin->Id());
    auto profit = value - cost;
    printf("%28s %5s  %28s  %28s  %28s\n", amount.ToStr().c_str(),
        coin->Symbol().c_str(), cost.ToStr().c_str(), value.ToStr().c_str(),
        profit.ToStr().c_str());
  }
  printf("\n\n");
}

void Taxes::PrintGainLoss(
    std::vector<GainLoss>* gains, bool fuse, Datetime from) const {
  printf("%34s  %10s  %10s  %28s  %28s  %28s\n", "Description", "Acquired",
      "Disposed", "Net Proceeds (USD)", "Net Cost (USD)", "Profit/Loss (USD)");

  if (gains->size() == 0) return;

  std::sort(
      gains->begin(), gains->end(), [](const GainLoss& a, const GainLoss& b) {
        if (a.coin->Id() == b.coin->Id()) {
          if (a.disposed == b.disposed) {
            return a.acquired < b.acquired;
          } else {
            return a.disposed < b.disposed;
          }
        } else {
          return a.coin->Id() < b.coin->Id();
        }
      });

  std::vector<GainLoss> fused;
  std::vector<GainLoss>* fused_ptr;

  if (fuse) {
    fused_ptr = &fused;
    fused.push_back((*gains)[0]);

    for (size_t i = 1; i < gains->size(); ++i) {
      auto& g = (*gains)[i];
      auto& prev = fused[fused.size() - 1];

      // we can fuse this gain with the previous one if they have the same
      // coin and were disposed on the same day
      if ((prev.coin->Id() == g.coin->Id()) &&
          (prev.disposed.EndOfDay() == g.disposed.EndOfDay())) {
        prev.amount += g.amount;
        prev.proceeds += g.proceeds;
        prev.cost += g.cost;
        if (prev.acquired != g.acquired) prev.various_acquired_dates = true;
      } else {
        fused.push_back(g);
      }
    }
  } else {
    fused_ptr = gains;
  }

  // now sort again this time by disposed date first
  std::sort(fused_ptr->begin(), fused_ptr->end(),
      [](const GainLoss& a, const GainLoss& b) {
        if (a.disposed == b.disposed) {
          return a.coin->Id() < b.coin->Id();
        } else {
          return a.disposed < b.disposed;
        }
      });

  for (auto& g : *fused_ptr) {
    if (g.disposed < from) continue;

    Amount profit = g.proceeds - g.cost;
    printf("%28s %5s  %10s  %10s  %28s  %28s  %28s\n", g.amount.ToStr().c_str(),
        g.coin->Symbol().c_str(),
        g.various_acquired_dates ? "VARIOUS"
                                 : g.acquired.ToStrDayUTCIRS().c_str(),
        g.disposed.ToStrDayUTCIRS().c_str(), g.proceeds.ToStr().c_str(),
        g.cost.ToStr().c_str(), profit.ToStr().c_str());
  }
}
