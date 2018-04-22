#include "PriceSource.hpp"

#include <sstream>

#include <json/reader.h>

#include "File.hpp"

const std::string PriceSource::coin_list_url =
    "https://api.coinmarketcap.com/v1/ticker/?limit=0";

std::string PriceSource::DoGetURL(std::string url) const {
  curlpp::Easy req;
  req.setOpt(curlpp::options::Url(url));

  std::ostringstream os;
  req.setOpt(curlpp::options::WriteStream(&os));

  req.perform();
  return os.str();
}

void PriceSource::AddAllCoins(File* file) {
  auto json = PriceSource::GetURL(coin_list_url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + coin_list_url);

  for (auto& c : root) {
    Coin::Create(
        file, c["id"].asString(), c["name"].asString(), c["symbol"].asString());
  }
}

Amount PriceSource::GetFee(std::shared_ptr<const Coin> coin, std::string txn) {
  auto id = coin->Id();
  if ((id == "bitcoin") || (id == "dash") || (id == "litecoin") ||
      (id == "dogecoin")) {
    // use chain.so
    std::string url =
        "https://chain.so/api/v2/tx/" + coin->Symbol() + "/" + txn;
    auto json = PriceSource::GetURL(url);

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(json, root)) {
      printf("WARNING: Could not parse result from %s\n", url.c_str());
      return 0;
    }

    if (root["status"].asString() != "success") {
      printf("WARNING: API request failed: %s\n", url.c_str());
      return 0;
    }

    return Amount::Parse(root["data"]["fee"].asString());
  } else {
    printf("WARNING: Don't know how to get fee of coin '%s'\n", id.c_str());
    return 0;
  }
}

std::unordered_map<std::string, Amount> PriceSource::GetUSDPrices() {
  auto json = PriceSource::GetURL(coin_list_url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + coin_list_url);

  std::unordered_map<std::string, Amount> prices;
  prices["us-dollar"] = 1;

  for (auto& c : root) {
    std::string price = "0";
    if (!c["price_usd"].isNull()) price = c["price_usd"].asString();

    prices[c["id"].asString()] = Amount::Parse(price);
  }

  return prices;
}
