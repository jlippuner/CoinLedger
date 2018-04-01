#include "File.hpp"

int main(int, char**) {
  auto f = File::InitNewFile();

  // for (const auto& c : f.Coins()) {
  //   printf("%s\n", c.first.c_str());
  // }

  f.Save("test.sqlite3");
  return 0;
}