/// \file Split.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Split.hpp"

#include "File.hpp"

std::shared_ptr<Split> Split::Create(File * file,
    std::shared_ptr<const Transaction> transaction,
    std::shared_ptr<const Account> account, std::string memo, Amount amount,
    std::shared_ptr<const Coin> coin) {
  auto id = uuid_t::Random();
  return file->AddSplit(Split(id, transaction, account, memo, amount,
      coin));
}
