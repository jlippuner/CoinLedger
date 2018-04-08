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
