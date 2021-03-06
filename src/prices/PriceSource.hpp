#ifndef SRC_PRICES_PRICESOURCE_HPP_
#define SRC_PRICES_PRICESOURCE_HPP_

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include "Amount.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"

class File;

class PriceSource {
 public:
  static PriceSource& Instance() {
    static PriceSource p;
    return p;
  }

  ~PriceSource() { curlpp::terminate(); }

  static std::string GetCoinMarketCapURL();

  static std::string GetURL(std::string url) {
    return Instance().DoGetURL(url);
  }

  static void AddAllCoins(File* file, bool unique_symbol = false);

  static Amount GetFee(std::shared_ptr<const Coin> coin, std::string txn);

  static std::unordered_map<std::string, Amount> GetUSDPrices();

  static std::unordered_map<std::string, int> GetNumIds();

 private:
  PriceSource() { curlpp::initialize(); }

  std::string DoGetURL(std::string url) const;
};

#endif  // SRC_PRICES_PRICESOURCE_HPP_