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

#include <boost/uuid/uuid.hpp>

#include "Coin.hpp"

class Account {
public:
  // no constructors that don't create a new id
  Account() = delete;
  Account(const Account&) = delete;
  Account& operator=(const Account&) = delete;

private:
  // unique global identifier of this account
  const boost::uuids::uuid id_;

  // name of this account
  std::string name_;

  // true if this is a placeholder account that doesn't have any transactions
  // but only serves as a parent for other accounts
  bool placeholder_;

  // the parent of this account, the only accounts that have no parent are the
  // special Asset, Liability, Income, Expense, Equity accounts
  const Account * parent_;

  // child accounts whose parent account is this account
  std::vector<const Account*> children_;

  // true if this account only has transactions in a single coin
  bool single_coin_;

  // if this is a single coin account, this is the coin used in this account
  const Coin * coin_;
};

#endif // SRC_ACCOUNT_HPP_
