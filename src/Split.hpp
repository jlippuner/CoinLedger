/// \file Split.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_SPLIT_HPP_
#define SRC_SPLIT_HPP_

#include <memory>

#include "Amount.hpp"
#include "Coin.hpp"
#include "UUID.hpp"

class Account;
class Transaction;

// this contains the split information without an id and transaction id, in
// order to perform error checking before the actual split gets created
class ProtoSplit {
public:
  ProtoSplit(std::shared_ptr<const Account> account, std::string memo,
      Amount amount, std::shared_ptr<const Coin> coin, std::string import_id):
      account_(account),
      memo_(memo),
      amount_(amount),
      coin_(coin),
      import_id_(import_id) {}

  // the account to or from which the amount is added or subtracted
  std::shared_ptr<const Account> account_;

  // memo of this split
  std::string memo_;

  // the amount of this split, always positive
  Amount amount_;

  // the coin that is credited or debited to the account
  std::shared_ptr<const Coin> coin_;

  // if this split is imported from an external source, an import ID can
  // be stored here in order to avoid duplicate imports
  std::string import_id_;
};

class Split {
public:
  static std::shared_ptr<Split> Create(File * file,
      std::shared_ptr<const Transaction> transaction,
      std::shared_ptr<const Account> account, std::string memo, Amount amount,
      std::shared_ptr<const Coin> coin, std::string import_id);

  static std::shared_ptr<Split> Create(File * file,
      std::shared_ptr<const Transaction> transaction,
      const ProtoSplit& protoSplit) {
    return Split::Create(file, transaction, protoSplit.account_,
        protoSplit.memo_, protoSplit.amount_, protoSplit.coin_,
        protoSplit.import_id_);
  }

  uuid_t Id() const { return id_; }
  std::shared_ptr<const Transaction> GetTransaction() const {
      return transaction_;
  }
  std::shared_ptr<const Account> GetAccount() const { return account_; }
  const std::string& Memo() const { return memo_; }
  Amount GetAmount() const { return amount_; }
  std::shared_ptr<const Coin> GetCoin() const { return coin_; }
  const std::string& Import_id() const { return import_id_; }

private:
  friend class File;

  Split(uuid_t id, std::shared_ptr<const Transaction> transaction,
      std::shared_ptr<const Account> account, std::string memo, Amount amount,
      std::shared_ptr<const Coin> coin, std::string import_id);

  // unique global identifier of this split
  const uuid_t id_;

  // the transaction with which this split is associated
  std::shared_ptr<const Transaction> const transaction_;

  // the account to or from which the amount is added or subtracted
  std::shared_ptr<const Account> account_;

  // memo of this split
  std::string memo_;

  // the amount of this split, always positive
  Amount amount_;

  // the coin that is credited or debited to the account
  std::shared_ptr<const Coin> coin_;

  // if this split is imported from an external source, an import ID can
  // be stored here in order to avoid duplicate imports
  std::string import_id_;
};

#endif // SRC_SPLIT_HPP_
