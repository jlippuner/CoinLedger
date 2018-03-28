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

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#include "Account.hpp"
#include "Coin.hpp"
#include "Split.hpp"
#include "Transaction.hpp"
#include "UUID.hpp"

template<typename T>
using UUIDMap = std::unordered_map<uuid_t, T, boost::hash<uuid_t>>;

// This class represents a CoinLedger file that stores all the information
// contained in the program. The actual file used to write to and read from is
// a SQLite database file.
// When a file is opened, all the information is copied into memory and
// manipulated in memory. Only when the file is explicitly saved is all the data
// written back to the file.

class File {
public:
  static File InitNewFile();

  static File Open(const std::string path);

  void Save(const std::string path) const;

  boost::uuids::uuid GetUUID() {
    return generator_();
  }

  void AddCoin(Coin && coin) {
    coins_.emplace(coin.Id(), std::move(coin));
  }

  std::unordered_map<std::string, Coin>& Coins() {
    return coins_;
  }

  const std::unordered_map<std::string, Coin>& Coins() const {
    return coins_;
  }

  void AddAccount(Account && account) {
    // do this first! after moving account, the full name and id will no
    // longer be available
    accounts_by_fullname_.insert({{ account.FullName(), account.Id() }});

    accounts_.emplace(account.Id(), std::move(account));
  }

  UUIDMap<Account>& Accounts() {
    return accounts_;
  }

  const UUIDMap<Account>& Accounts() const {
    return accounts_;
  }

  const std::unordered_map<std::string, uuid_t>& Accounts_by_fullname() const {
    return accounts_by_fullname_;
  }

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

  // random generator to generate new UUID's
  boost::uuids::random_generator generator_;
};

#endif // SRC_FILE_HPP_
