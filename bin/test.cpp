#include "PriceSource.hpp"

int main(int, char**) {
  auto res = PriceSource::GetURL(
    "https://api.coinmarketcap.com/v1/ticker/?limit=0");

  printf("%s\n", res.c_str());

  return 0;
}