/// \file Account.cpp
/// \author jlippuner
/// \since Mar 16, 2018
///
/// \brief
///
///

#include "Account.hpp"

#include "File.hpp"

Account* Account::Create(File* file, std::string name, bool placeholder,
      const Account * parent, bool single_coin, const Coin * coin) {
  // check if an account with this full name already exists
  auto full_name = MakeFullName(parent, name);

  if (file->Accounts_by_fullname().count(full_name) > 0) {
    printf("WARNING account '%s' already exists\n", full_name.c_str());
    return nullptr;
  } else {
    auto id = file->GetUUID();
    file->AddAccount(Account(id, name, placeholder, parent, single_coin, coin));
    return &file->Accounts().at(id);
  }
}

std::string Account::MakeFullName(const Account * parent, std::string name) {
  if (parent == nullptr) {
    return name;
  } else {
    return parent->FullName() + "::" + name;
  }
}
