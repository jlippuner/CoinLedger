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

// this contains the split information without an id and transaction id, in
// order to perform error checking before the actual split gets created
class ProtoSplit {
public:
  ProtoSplit(const Account * account, std::string memo, Amount amount,
      const Coin * coin):
      account_(account),
      memo_(memo),
      amount_(amount),
      coin_(coin) {}

  // the account to or from which the amount is added or subtracted
  const Account * account_;

  // memo of this split
  std::string memo_;

  // the amount of this split, always positive
  Amount amount_;

  // the coin that is credited or debited to the account
  const Coin * coin_;
};

class Split {
public:
  static Split* Create(File* file, const Transaction * transaction,
      const Account * account, std::string memo, Amount amount,
      const Coin * coin);

  static Split* Create(File* file, const Transaction * transaction,
      const ProtoSplit& protoSplit) {
    return Split::Create(file, transaction, protoSplit.account_,
        protoSplit.memo_, protoSplit.amount_, protoSplit.coin_);
  }

  uuid_t Id() const { return id_; }
  const Transaction * GetTransaction() const { return transaction_; }
  const Account * GetAccount() const { return account_; }
  const std::string& Memo() const { return memo_; }
  Amount GetAmount() const { return amount_; }
  const Coin * GetCoin() const { return coin_; }

private:
  friend class File;

  Split(uuid_t id, const Transaction * transaction, const Account * account,
      std::string memo, Amount amount, const Coin * coin):
      id_(id),
      transaction_(transaction),
      account_(account),
      memo_(memo),
      amount_(amount),
      coin_(coin) {}

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
