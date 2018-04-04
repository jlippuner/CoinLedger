/// \file Transaction.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Transaction.hpp"

#include "File.hpp"

Transaction* Transaction::Create(File* file, Datetime date,
    std::string description, const std::vector<ProtoSplit>& protoSplits,
    std::string import_id) {
  // check the proto splits to make sure none have a 0 amount and there is at
  // least one negative amount and one positive amount
  bool negative = false;
  bool positive = false;
  for (auto& s : protoSplits) {
    if (s.amount_ == 0) {
      printf("ERROR: Got split with 0 amount\n");
      return nullptr;
    }
    if (s.amount_ > 0)
      positive = true;
    else
      negative = true;

    if (s.account_ == nullptr) {
      printf("ERROR: Got split with no account\n");
      return nullptr;
    }

    if (s.coin_ == nullptr) {
      printf("ERROR: Got split with no coin");
      return nullptr;
    }
  }

  if (!(positive && negative)) {
    printf("ERROR: Transaction must have at least one positive and one "
        "negative split");
    return nullptr;
  }

  // all splits are ok at this point, create the transaction
  auto transaction_id = uuid_t::Random();
  auto transaction = file->AddTransaction(Transaction(transaction_id, date,
      description, import_id));

  // now create the splits and add them to the transaction
  for (auto& s : protoSplits) {
    auto split = Split::Create(file, transaction, s);
    transaction->AddSplit(split);
  }

  return transaction;
}
