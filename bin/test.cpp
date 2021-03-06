#include <iostream>

#include "Amount.hpp"
#include "File.hpp"
#include "importers/CSV.hpp"
#include "importers/GDAX.hpp"

int main(int, char**) {
  // auto f = File::InitNewFile();
  // f.Save("test.sqlite3");

  // auto f = File::Open("default.sqlite3");
  // f.PrintAccountTree();

  // auto a = f.GetAccount("Assets::Wallets::Bitcoin Core");
  // printf("%s\n", a->FullName().c_str());

  // for (const auto& c : f.Coins()) {
  //   printf("%s: %s (%s)\n", c.first.c_str(), c.second.Symbol().c_str(),
  //       c.second.Name().c_str());
  // }

  // for (const auto& itm : f.Accounts()) {
  //   const Account& acc = itm.second;
  //   std::cout << acc.Id() << " is account " << acc.Name() << std::endl;
  // }

  // CSV csv("/home/jlippuner/MEGA/finances/crypto/GDAX_USD_account.csv");

  // for (auto& r : csv.Content()) {
  //   auto amt = r[2];
  //   auto a = Amount::Parse(amt);
  //   printf("%25s  %20li  %25s\n", amt.c_str(), a.Raw(), a.ToStr().c_str());
  // }

  auto amt = Amount::Parse("42.0005");
  printf("Amount size: %lu\n", Amount::size());
  printf("Datetime size: %lu\n", Datetime::size());

  printf("amt = %s\n", amt.ToStr().c_str());

  void* dat = (void*)malloc(Amount::size());
  memcpy(dat, amt.Raw(), Amount::size());

  auto amt2 = Amount::FromRaw(dat);
  printf("amt2 = %s\n", amt2.ToStr().c_str());

  return 0;
}