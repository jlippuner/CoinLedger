/// \file Split.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Split.hpp"

#include "Account.hpp"
#include "File.hpp"

Split::Split(uuid_t id, std::shared_ptr<const Transaction> transaction,
    std::shared_ptr<const Account> account, std::string memo, Amount amount,
    std::shared_ptr<const Coin> coin, std::string import_id)
    : id_(id),
      transaction_(transaction),
      account_(account),
      memo_(memo),
      amount_(amount),
      coin_(coin),
      import_id_(import_id) {
  if (account->Placeholder()) {
    throw std::invalid_argument(
        "Can't add a split to placeholder account " + account->FullName());
  }

  if (account->SingleCoin()) {
    // make sure the coin of this split matches the coin of the account
    if (account->GetCoin()->Id() != coin->Id()) {
      throw std::invalid_argument(
          "Can't add a split with coin " + coin->Name() + " to account " +
          account->FullName() + " that only holds coin " +
          account->GetCoin()->Name());
    }
  }
}

std::shared_ptr<Split> Split::Create(File* file,
    std::shared_ptr<const Transaction> transaction,
    std::shared_ptr<const Account> account, std::string memo, Amount amount,
    std::shared_ptr<const Coin> coin, std::string import_id) {
  auto id = uuid_t::Random();
  return file->AddSplit(
      Split(id, transaction, account, memo, amount, coin, import_id));
}
