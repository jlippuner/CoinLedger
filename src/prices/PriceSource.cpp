#include "PriceSource.hpp"

#include <chrono>
#include <sstream>
#include <thread>

#include <json/reader.h>

#include "Amount.hpp"
#include "File.hpp"

std::string PriceSource::GetCoinMarketCapURL() {
  std::string url =
      "https://pro-api.coinmarketcap.com/v1/cryptocurrency/listings/latest";
  url += "?start=1&limit=5000&convert=USD";
  url += "&CMC_PRO_API_KEY=";

  auto key = std::getenv("COINMARKETCAP_API_KEY");
  if (key == nullptr) {
    printf(
        "ERROR: No CoinMarketCap.com API key found. Please set the "
        "COINMARKETCAP_API_KEY environment variable.\n");
    exit(EXIT_FAILURE);
  }
  url += std::string(key);
  // printf("URL = %s\n", url.c_str());

  return url;
}

std::string PriceSource::DoGetURL(std::string url) const {
  curlpp::Easy req;
  req.setOpt(curlpp::options::Url(url));

  std::ostringstream os;
  req.setOpt(curlpp::options::WriteStream(&os));

  req.perform();
  return os.str();
}

void PriceSource::AddAllCoins(File* file) {
  auto coin_list_url = GetCoinMarketCapURL();
  auto json = PriceSource::GetURL(coin_list_url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + coin_list_url);

  for (auto& c : root["data"]) {
    Coin::Create(
        file, c["slug"].asString(), c["name"].asString(), c["symbol"].asString());
  }
}

Amount PriceSource::GetFee(std::shared_ptr<const Coin> coin, std::string txn) {
  if (txn == "feed930e29695ad948e0a2b92ed818e283cdafaffa22334d8a9b27b32071050f")
    return Amount::Parse("0.00000849");
  if (txn == "ace3a7e9a8104604383ebc7b86a73174653052bf22d524e71850c637557525fe")
    return Amount::Parse("0.00028458");
  if (txn == "8f0341b0e60bc74a2bb89be91f48f3a94e4efa11c0f671289f0229566e56b40f")
    return Amount::Parse("0.00000398");
  if (txn == "131acc9afd350a4bb3fc43b71dc0c64a1cac5a80e401f4a8a7e80a98b7a34bda")
    return Amount::Parse("0.00000429");
  // if (txn == "")
  //   return Amount::Parse("");

  auto id = coin->Id();
  if ((id == "bitcoin") || (id == "dash") || (id == "litecoin") ||
      (id == "dogecoin")) {
    // use chain.so
    std::string url =
        "https://chain.so/api/v2/tx/" + coin->Symbol() + "/" + txn;
    auto json = PriceSource::GetURL(url);

    Json::Reader reader;
    Json::Value root;
    while (!reader.parse(json, root)) {
      printf("WARNING: Could not parse result from %s, trying again in 5 sec\n", url.c_str());
      printf("res: %s\n", json.c_str());

      std::this_thread::sleep_for(std::chrono::seconds(5));
      json = PriceSource::GetURL(url);
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
  auto coin_list_url = GetCoinMarketCapURL();
  auto json = PriceSource::GetURL(coin_list_url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + coin_list_url);

  std::unordered_map<std::string, Amount> prices;
  prices[Coin::USD_id()] = 1;

  for (auto& c : root["data"]) {
    std::string price = "0";

    if (!c["quote"].isNull() && !c["quote"]["USD"].isNull() &&
        !c["quote"]["USD"]["price"].isNull()) {
      auto p = c["quote"]["USD"]["price"].asDouble();
      price = std::to_string(p);
    }

    prices[c["slug"].asString()] = Amount::Parse(price);
  }

  // hacks for old currencies or ids
  prices["modum"] = Amount(0.181212);
  prices["vivo"] = Amount(0.0143);
  prices["segwit2x"] = Amount(5.3259);
  prices["viuly"] = Amount(0.0);
  prices["xenon"] = Amount(0.0);

  prices["ripple"] = prices["xrp"];

  return prices;
}
