/// \file Split.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Split.hpp"

#include "File.hpp"

Split* Split::Create(File* file, const Transaction * transaction,
    const Account * account, std::string memo, Amount amount,
    const Coin * coin) {
  auto id = file->GetUUID();
  return file->AddSplit(Split(id, transaction, account, memo, amount,
      coin));
}
