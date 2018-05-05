/// \file Transaction.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Transaction.hpp"

#include "File.hpp"

std::shared_ptr<Transaction> Transaction::Create(File* file, Datetime date,
    std::string description, const std::vector<ProtoSplit>& protoSplits,
    std::string import_id) {
  // check the proto splits to make sure a coin is set and the amount is
  // non-zero, allow unmatched splits
  for (auto& s : protoSplits) {
    if (s.amount_ == 0) {
      printf("ERROR: Got split with 0 amount\n");
      return nullptr;
    }

    if (s.account_ == nullptr) {
      printf("ERROR: Got split with no account\n");
      return nullptr;
    }

    if (s.coin_ == nullptr) {
      printf("ERROR: Got split with no coin");
      return nullptr;
    }
  }

  // all splits are ok at this point, create the transaction
  auto transaction_id = uuid_t::Random();
  auto transaction = file->AddTransaction(
      Transaction(transaction_id, date, description, import_id));

  // now create the splits and add them to the transaction
  for (auto& s : protoSplits) {
    auto split = Split::Create(file, transaction, s);
    transaction->AddSplit(split);
  }

  return transaction;
}

bool Transaction::Matched() const {
  bool positive = false;
  bool negative = false;

  for (auto s : splits_) {
    if (s->GetAmount() > 0) positive = true;
    if (s->GetAmount() < 0) negative = true;
  }

  return (positive && negative);
}

bool Transaction::Balanced() const {
  if (!Matched()) return false;

  if (splits_.size() < 2) return false;

  auto coin = GetCoin();

  if (coin != nullptr) {
    // make sure all the amounts add up to 0
    Amount sum = 0;
    for (auto& s : splits_) sum += s->GetAmount();

    if (sum != 0) return false;
  }

  return true;
}

bool Transaction::HasSplitWithImportId(const std::string& import_id) const {
  for (auto& s : splits_) {
    if (s->Import_id() == import_id) return true;
  }
  return false;
}

std::shared_ptr<const Coin> Transaction::GetCoin() const {
  auto coin = splits_[0]->GetCoin();
  for (size_t i = 1; i < splits_.size(); ++i) {
    if (splits_[i]->GetCoin()->Id() != coin->Id()) {
      return nullptr;
    }
  }

  return coin;
}

void Transaction::Print(const bool print_import_id) const {
  auto desc = Description();
  if (!Matched())
    desc = "[UNMATCHED] " + desc;
  else if (!Balanced())
    desc = "[UNBALANCED] " + desc;

  printf("%s %s\n", Date().ToStrUTC().c_str(), desc.c_str());
  if (print_import_id) printf("  %s\n", Import_id().c_str());

  for (auto& s : Splits()) {
    printf("  %18s %s to %s (%s)\n", s->GetAmount().ToStr().c_str(),
        s->GetCoin()->Symbol().c_str(), s->GetAccount()->FullName().c_str(),
        s->Memo().c_str());
  }
  printf("\n");
}
