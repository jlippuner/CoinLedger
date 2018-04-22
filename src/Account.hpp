/// \file Account.hpp
/// \author jlippuner
/// \since Mar 16, 2018
///
/// \brief
///
///

#ifndef SRC_ACCOUNT_HPP_
#define SRC_ACCOUNT_HPP_

#include <memory>
#include <string>
#include <vector>

#include "Balance.hpp"
#include "Coin.hpp"
#include "UUID.hpp"

class File;

class Account {
 public:
  static std::shared_ptr<Account> Create(File* file, std::string name,
      bool placeholder, std::shared_ptr<Account> parent, bool single_coin,
      std::shared_ptr<const Coin> coin = nullptr);

  uuid_t Id() const { return id_; }
  const std::string& Name() const { return name_; }
  bool Placeholder() const { return placeholder_; }
  std::shared_ptr<const Account> Parent() const { return parent_; }
  bool SingleCoin() const { return single_coin_; }
  std::shared_ptr<const Coin> GetCoin() const { return coin_; }

  static std::string MakeFullName(
      std::shared_ptr<const Account> parent, std::string name);

  std::string FullName() const { return MakeFullName(parent_, name_); }

  void PrintTree(std::string indent = "") const;

  // print the balance of this account, all the sub account balance trees, and
  // then print the total balance in this account and return the total balance
  Balance PrintTreeBalance(const UUIDMap<Balance>& balances, std::string indent,
      bool flip_sign,
      const std::unordered_map<std::string, Amount>* prices) const;

 private:
  friend class File;

  Account(uuid_t id, std::string name, bool placeholder,
      std::shared_ptr<const Account> parent, bool single_coin,
      std::shared_ptr<const Coin> coin = nullptr)
      : id_(id),
        name_(name),
        placeholder_(placeholder),
        parent_(parent),
        single_coin_(single_coin),
        coin_(coin) {}

  void SetParent(std::shared_ptr<Account> parent) {
    parent_ = parent;
    // parent->AddChild(this);
  }

  void AddChild(std::shared_ptr<const Account> child) {
    children_.push_back(child);
  }

  // unique global identifier of this account
  const uuid_t id_;

  // name of this account
  std::string name_;

  // true if this is a placeholder account that doesn't have any transactions
  // but only serves as a parent for other accounts
  bool placeholder_;

  // the parent of this account, the only accounts that have no parent are the
  // special Asset, Liability, Income, Expense, Equity accounts
  std::shared_ptr<const Account> parent_;

  // true if this account only has transactions in a single coin
  bool single_coin_;

  // if this is a single coin account, this is the coin used in this account
  std::shared_ptr<const Coin> coin_;

  // child accounts whose parent account is this account
  mutable std::vector<std::shared_ptr<const Account>> children_;
};

#endif  // SRC_ACCOUNT_HPP_
