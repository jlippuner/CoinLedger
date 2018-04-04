#include "File.hpp"

#include <iostream>


int main(int, char**) {
  // auto f = File::InitNewFile();
  // f.Save("test.sqlite3");

  auto f = File::Open("default.sqlite3");
  f.PrintAccountTree();

  auto a = f.GetAccount("Assets::Wallets::Bitcoin Core");
  printf("%s\n", a->FullName().c_str());

  // for (const auto& c : f.Coins()) {
  //   printf("%s: %s (%s)\n", c.first.c_str(), c.second.Symbol().c_str(),
  //       c.second.Name().c_str());
  // }

  // for (const auto& itm : f.Accounts()) {
  //   const Account& acc = itm.second;
  //   std::cout << acc.Id() << " is account " << acc.Name() << std::endl;
  // }

  return 0;
}