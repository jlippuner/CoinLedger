/// \file Account.cpp
/// \author jlippuner
/// \since Mar 16, 2018
///
/// \brief
///
///

#include "Account.hpp"

#include "File.hpp"

Account& Account::Create(File* file, std::string name, bool placeholder,
      const Account * parent, bool single_coin, const Coin * coin) {
  auto id = file->GetUUID();
  file->AddAccount(Account(id, name, placeholder, parent, single_coin, coin));
  return file->Accounts().at(id);
}
