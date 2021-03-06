/// \file File.hpp
/// \author jlippuner
/// \since Mar 21, 2018
///
/// \brief
///
///

#ifndef SRC_FILE_HPP_
#define SRC_FILE_HPP_

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>

#include "Account.hpp"
#include "Balance.hpp"
#include "Coin.hpp"
#include "Split.hpp"
#include "Transaction.hpp"
#include "UUID.hpp"

#include "prices/DailyData.hpp"
#include "prices/PriceSource.hpp"

// This class represents a CoinLedger file that stores all the information
// contained in the program. The actual file used to write to and read from is
// a SQLite database file.
// When a file is opened, all the information is copied into memory and
// manipulated in memory. Only when the file is explicitly saved is all the data
// written back to the file.

class File {
 public:
  static File InitNewFile();

  static File Open(const std::string& path);

  void Save(const std::string& path) const;

  // get the transaction with the given import id, if there is no such
  // transaction, return nullptr, unless fail_if_not_exist is true, in which
  // case an exception is thrown; if there are multiple transactions with this
  // import id, throw an exception
  std::shared_ptr<Transaction> GetTransactionFromImportId(
      const std::string& import_id, bool fail_if_not_exist = false);

  // balance the unbalanced, single-coin transaction identified by the import id
  // by adding a split to the given account that will balance the transaction
  void BalanceTransaction(
      const std::string& txn_import_id, std::shared_ptr<const Account> account);

  void PrintAccountTree() const;

  void PrintTransactions() const;
  void PrintUnbalancedTransactions() const;
  void PrintUnmatchedTransactions() const;

  UUIDMap<Balance> MakeAccountBalances() const;

  void PrintAccountBalances(bool fetch_usd_prices = true) const;

  Amount GetHistoricUSDPrice(
      Datetime time, std::shared_ptr<const Coin> coin) const;

  void AddNewCoins() { PriceSource::AddAllCoins(this, true); }

  void AddCoinNumIds();

  std::shared_ptr<Coin> AddCoin(const Coin& coin) {
    auto res =
        coins_.emplace(coin.Id(), std::make_shared<Coin>(coin)).first->second;
    coin_by_symbol_.insert({{res->Symbol(), res}});
    return res;
  }
  std::shared_ptr<Coin> GetCoin(std::string id) {
    try {
      return coins_.at(id);
    } catch (std::exception& e) {
      printf("There is no coin with id '%s'\n", id.c_str());
      throw e;
    }
  }
  std::shared_ptr<const Coin> GetCoin(std::string id) const {
    try {
      return coins_.at(id);
    } catch (std::exception& e) {
      printf("There is no coin with id '%s'\n", id.c_str());
      throw e;
    }
  }
  std::shared_ptr<const Coin> GetCoinBySymbol(std::string symbol) const;
  const std::unordered_map<std::string, std::shared_ptr<Coin>>& Coins() const {
    return coins_;
  }
  const std::unordered_multimap<std::string, std::shared_ptr<Coin>>&
  CoinBySymbol() const {
    return coin_by_symbol_;
  }

  std::shared_ptr<Account> AddAccount(const Account& account) {
    auto res =
        accounts_.emplace(account.Id(), std::make_shared<Account>(account))
            .first->second;
    accounts_by_fullname_.insert({{account.FullName(), res}});
    return res;
  }
  std::shared_ptr<Account> GetAccount(uuid_t id) { return accounts_.at(id); }
  const UUIDMap<std::shared_ptr<Account>>& Accounts() const {
    return accounts_;
  }

  const std::unordered_map<std::string, std::shared_ptr<Account>>&
  AccountsByFullname() const {
    return accounts_by_fullname_;
  }
  std::shared_ptr<Account> GetAccount(std::string fullname) {
    return accounts_by_fullname_.at(fullname);
  }
  std::shared_ptr<const Account> GetAccount(std::string fullname) const {
    return accounts_by_fullname_.at(fullname);
  }

  std::shared_ptr<Transaction> AddTransaction(const Transaction& transaction) {
    auto res = transactions_
                   .emplace(transaction.Id(),
                       std::make_shared<Transaction>(transaction))
                   .first->second;
    transactions_by_import_id_.insert({{res->Import_id(), res}});
    return res;
  }
  std::shared_ptr<Transaction> GetTransaction(uuid_t id) {
    return transactions_.at(id);
  }
  const UUIDMap<std::shared_ptr<Transaction>>& Transactions() const {
    return transactions_;
  }
  const std::unordered_multimap<std::string, std::shared_ptr<Transaction>>&
  TransactionsByImportId() const {
    return transactions_by_import_id_;
  }

  std::shared_ptr<Split> AddSplit(const Split& split) {
    return splits_.emplace(split.Id(), std::make_shared<Split>(split))
        .first->second;
  }
  std::shared_ptr<Split> GetSplit(uuid_t id) { return splits_.at(id); }
  const UUIDMap<std::shared_ptr<Split>>& Splits() const { return splits_; }

 private:
  File() {}

  void PrintTransactions(std::vector<std::shared_ptr<Transaction>> txns,
      bool print_import_id = false) const;

  // all the known coins
  std::unordered_map<std::string, std::shared_ptr<Coin>> coins_;
  // coin symbols are mostly unique, but not always
  std::unordered_multimap<std::string, std::shared_ptr<Coin>> coin_by_symbol_;

  // historical daily price data
  mutable std::unordered_map<std::string, DailyData> daily_data_;

  // all accounts
  UUIDMap<std::shared_ptr<Account>> accounts_;

  // map of full account names (parent::parent::account) vs. account id
  std::unordered_map<std::string, std::shared_ptr<Account>>
      accounts_by_fullname_;

  // all transactions
  UUIDMap<std::shared_ptr<Transaction>> transactions_;

  // transactions by import id
  std::unordered_multimap<std::string, std::shared_ptr<Transaction>>
      transactions_by_import_id_;

  // all std::shared_ptrlits
  UUIDMap<std::shared_ptr<Split>> splits_;
};

#endif  // SRC_FILE_HPP_
