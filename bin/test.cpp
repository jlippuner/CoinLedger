#include "File.hpp"

#include <iostream>
#include <boost/uuid/uuid_io.hpp>

int main(int, char**) {
  //auto f = File::InitNewFile();
  //f.Save("test.sqlite3");

  auto f = File::Open("test.sqlite3");

  // for (const auto& c : f.Coins()) {
  //   printf("%s: %s (%s)\n", c.first.c_str(), c.second.Symbol().c_str(),
  //       c.second.Name().c_str());
  // }

  for (const auto& itm : f.Accounts()) {
    const Account& acc = itm.second;
    std::cout << acc.Id() << " is account " << acc.Name() << std::endl;
  }

  return 0;
}