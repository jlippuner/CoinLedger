/// \file Account.hpp
/// \author jlippuner
/// \since Mar 16, 2018
///
/// \brief
///
///

#ifndef SRC_ACCOUNT_HPP_
#define SRC_ACCOUNT_HPP_

#include <string>
#include <vector>

#include "Coin.hpp"
#include "UUID.hpp"

class File;

class Account {
public:
  // no constructors that don't create a new id
  Account() = delete;
  Account(const Account&) = delete;
  Account& operator=(const Account&) = delete;

  Account(Account&&) = default;

  static Account* Create(File* file, std::string name, bool placeholder,
      const Account * parent, bool single_coin,
      const Coin * coin = nullptr);

  uuid_t Id() const { return id_; }
  const std::string& Name() const { return name_; }
  bool Placeholder() const { return placeholder_; }
  const Account * Parent() const { return parent_; }
  bool Single_coin() const { return single_coin_; }
  const Coin * GetCoin() const { return coin_; }

  static std::string MakeFullName(const Account * parent, std::string name);

  std::string FullName() const {
    return MakeFullName(parent_, name_);
  }

private:
  Account(uuid_t id, std::string name, bool placeholder, const Account * parent,
      bool single_coin, const Coin * coin = nullptr) :
      id_(id),
      name_(name),
      placeholder_(placeholder),
      parent_(parent),
      single_coin_(single_coin),
      coin_(coin) { }

  // unique global identifier of this account
  const uuid_t id_;

  // name of this account
  std::string name_;

  // true if this is a placeholder account that doesn't have any transactions
  // but only serves as a parent for other accounts
  bool placeholder_;

  // the parent of this account, the only accounts that have no parent are the
  // special Asset, Liability, Income, Expense, Equity accounts
  const Account * parent_;

  // true if this account only has transactions in a single coin
  bool single_coin_;

  // if this is a single coin account, this is the coin used in this account
  const Coin * coin_;

  // child accounts whose parent account is this account
  std::vector<const Account*> children_;

};

#endif // SRC_ACCOUNT_HPP_
