#ifndef SRC_PRICESOURCE_HPP_
#define SRC_PRICESOURCE_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include "Amount.hpp"
#include "Coin.hpp"

class File;

class PriceSource {
 public:
  static const std::string coin_list_url;

  static PriceSource& Instance() {
    static PriceSource p;
    return p;
  }

  ~PriceSource() { curlpp::terminate(); }

  static std::string GetURL(std::string url) {
    return Instance().DoGetURL(url);
  }

  static void AddAllCoins(File* file);

  static Amount GetFee(std::shared_ptr<const Coin> coin, std::string txn);

  static std::unordered_map<std::string, Amount> GetUSDPrices();

 private:
  PriceSource() { curlpp::initialize(); }

  std::string DoGetURL(std::string url) const;
};

#endif  // SRC_PRICESOURCE_HPP_