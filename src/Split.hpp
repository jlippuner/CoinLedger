/// \file Split.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_SPLIT_HPP_
#define SRC_SPLIT_HPP_

#include "Amount.hpp"
#include "Coin.hpp"
#include "UUID.hpp"

class Account;
class Transaction;

class Split {
public:
  // no constructs that don't create a new id
  Split() = delete;
  Split(const Split&) = delete;
  Split& operator=(const Split&) = delete;

  uuid_t Id() const { return id_; }
  const Transaction * GetTransaction() const { return transaction_; }
  const Account * GetAccount() const { return account_; }
  const std::string& Memo() const { return memo_; }
  Amount GetAmount() const { return amount_; }
  const Coin * GetCoin() const { return coin_; }

private:
  // unique global identifier of this split
  const uuid_t id_;

  // the transaction with which this split is associated
  const Transaction * const transaction_;

  // the account to or from which the amount is added or subtracted
  const Account * account_;

  // memo of this split
  std::string memo_;

  // the amount of this split, always positive
  Amount amount_;

  // the coin that is credited or debited to the account
  const Coin * coin_;
};

#endif // SRC_SPLIT_HPP_
