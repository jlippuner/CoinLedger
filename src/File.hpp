/// \file File.hpp
/// \author jlippuner
/// \since Mar 21, 2018
///
/// \brief
///
///

#ifndef SRC_FILE_HPP_
#define SRC_FILE_HPP_

#include <string>
#include <unordered_map>

#include "Account.hpp"
#include "Coin.hpp"
#include "Split.hpp"
#include "Transaction.hpp"
#include "UUID.hpp"

template<typename T>
using UUIDMap = std::unordered_map<uuid_t, T, uuid_t::hash>;

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

  Coin * AddCoin(Coin && coin) {
    return &coins_.emplace(coin.Id(), std::move(coin)).first->second;
  }
  Coin * GetCoin(std::string id) { return &coins_.at(id); }
  const std::unordered_map<std::string, Coin>& Coins() const { return coins_; }

  Account * AddAccount(Account && account) {
    // do this first! after moving account, the full name and id will no
    // longer be available
    accounts_by_fullname_.insert({{ account.FullName(), account.Id() }});

    return &accounts_.emplace(account.Id(), std::move(account)).first->second;
  }
  Account * GetAccount(uuid_t id) { return &accounts_.at(id); }
  const UUIDMap<Account>& Accounts() const { return accounts_; }

  const std::unordered_map<std::string, uuid_t>& Accounts_by_fullname() const {
    return accounts_by_fullname_;
  }
  uuid_t GetAccount_by_fullname(std::string fullname) const {
    return accounts_by_fullname_.at(fullname);
  }
  Account * GetAccount(std::string fullname) {
    return &accounts_.at(accounts_by_fullname_.at(fullname));
  }

  Transaction * AddTransaction(Transaction && transaction) {
    return &transactions_.emplace(transaction.Id(),
        std::move(transaction)).first->second;
  }
  Transaction * GetTransaction(uuid_t id) { return &transactions_.at(id); }
  const UUIDMap<Transaction>& Transactions() const { return transactions_; }

  Split * AddSplit(Split && split) {
    return &splits_.emplace(split.Id(), std::move(split)).first->second;
  }
  Split * GetSplit(uuid_t id) { return &splits_.at(id); }
  const UUIDMap<Split>& Splits() const { return splits_; }

private:
  File() {}

  // all the known coins
  std::unordered_map<std::string, Coin> coins_;

  // all accounts
  UUIDMap<Account> accounts_;

  // map of full account names (parent::parent::account) vs. account id
  std::unordered_map<std::string, uuid_t> accounts_by_fullname_;

  // all transactions
  UUIDMap<Transaction> transactions_;

  // all splits
  UUIDMap<Split> splits_;
};

#endif // SRC_FILE_HPP_
