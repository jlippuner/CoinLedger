/// \file Account.cpp
/// \author jlippuner
/// \since Mar 16, 2018
///
/// \brief
///
///

#include "Account.hpp"

#include "File.hpp"

std::shared_ptr<Account> Account::Create(File * file, std::string name,
    bool placeholder, std::shared_ptr<Account> parent,
    bool single_coin, std::shared_ptr<const Coin> coin) {
  // check if an account with this full name already exists
  auto full_name = MakeFullName(parent, name);

  if (file->AccountsByFullname().count(full_name) > 0) {
    printf("WARNING account '%s' already exists\n", full_name.c_str());
    return nullptr;
  } else {
    auto id = uuid_t::Random();
    auto new_account = file->AddAccount(Account(id, name, placeholder, parent,
        single_coin, coin));

    if (parent != nullptr)
      parent->AddChild(new_account);

    return new_account;
  }
}

std::string Account::MakeFullName(std::shared_ptr<const Account> parent,
    std::string name) {
  if (parent == nullptr) {
    return name;
  } else {
    return parent->FullName() + "::" + name;
  }
}

void Account::PrintTree(std::string indent) const {
  printf("%s%s\n", indent.c_str(), name_.c_str());

  // sort children by name
  std::sort(children_.begin(), children_.end(),
      [](std::shared_ptr<const Account> a, std::shared_ptr<const Account> b) {
        return a->name_ < b->name_;
      });

  for (auto c : children_)
    c->PrintTree(indent + "  ");
}
