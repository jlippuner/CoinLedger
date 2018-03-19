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

class Coin;

class Account {
public:


private:
  // unique global identifier of this account
  const size_t id_;

  // name of this account
  std::string name_;

  // true if this is a placeholder account that doesn't have any transactions
  // but only serves as a parent for other accounts
  bool placeholder_;

  // the parent of this account, the only accounts that have no parent are the
  // special Asset, Liability, Income, Expense, Equity accounts
  Account * parent_;

  // child accounts whose parent account is this account
  std::vector<Account*> children_;

  // true if this account only has transactions in a single coin
  bool single_coin_;

  // if this is a single coin account, this is the coin used in this account
  Coin * coin_;
};

#endif // SRC_ACCOUNT_HPP_
