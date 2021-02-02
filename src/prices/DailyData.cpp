#include "DailyData.hpp"

#include <htmlcxx/html/ParserDom.h>
#include <exception>
#include <thread>

#include <json/reader.h>
#include <json/writer.h>

#include "PriceSource.hpp"

Amount DailyData::operator()(const Datetime& date) {
  int64_t day = date.DailyDataDay();
  int64_t idx = day - start_day_;

  if ((idx >= 0) && (idx < (int64_t)prices_.size())) return prices_[idx];

  // we have to fetch more data
  int64_t from, to;
  if (idx < 0) {
    // add more data before the current prices
    from = day - 365;
    to = start_day_;
  } else {
    // add more data after the current prices
    if (start_day_ == 0)
      from = day - 365;
    else
      from = start_day_ + prices_.size() - 1;

    to = day + 365;
  }

  std::pair<std::vector<int64_t>, std::vector<Amount>> data;
  while (true) {
    try {
      data = GetData(from, to);
      break;
    } catch (std::exception& e) {
      printf("Failed to get data for %s, retrying in 30 seconds...\n",
          coin_->Id().c_str());
      std::this_thread::sleep_for(std::chrono::seconds(30));
    }
  }

  auto& days = data.first;
  auto& ps = data.second;
  size_t old_size = prices_.size();

  if (days.size() == 0) throw std::runtime_error("Got no data");

  // make sure the data we got doesn't have gaps
  for (size_t i = 1; i < days.size(); ++i) {
    if (days[i] != (days[i - 1] + 1))
      throw std::runtime_error("Got a gap in daily price data");
  }

  if (idx < 0) {
    // add more data before the current prices
    // make sure the last entry of the new data matches the first entry of the
    // existing data
    if (days[days.size() - 1] != start_day_)
      throw std::runtime_error("Unexpected last day of new data");

    if ((ps[ps.size() - 1] / prices_[0] - 1.0).Abs() > Amount::Parse("0.01")) {
      printf("Old price: %s, new price: %s\n",
          ps[ps.size() - 1].ToStr().c_str(), prices_[0].ToStr().c_str());
      throw std::runtime_error("Price mismatch between new and existing data");
    }

    start_day_ = days[0];
    prices_.insert(prices_.begin(), ps.begin(), ps.end() - 1);
  } else {
    // add more data after the current prices
    if (start_day_ == 0) {
      // we don't have any data yet, just use what we got
      start_day_ = days[0];
      prices_ = ps;
      old_size = 1;  // to make the check below pass
    } else {
      // make sure the last entry of the existing data matches the first entry
      // of the new data
      if (days[0] != (start_day_ + (int64_t)prices_.size() - 1))
        throw std::runtime_error("Unexpected first day of new data");

      if ((ps[0] / prices_[prices_.size() - 1] - 1.0).Abs() >
          Amount::Parse("0.01")) {
        printf("Old price: %s, new price: %s\n",
            prices_[prices_.size() - 1].ToStr().c_str(), ps[0].ToStr().c_str());
        throw std::runtime_error(
            "Price mismatch between new and existing data");
      }

      prices_.insert(prices_.end(), ps.begin() + 1, ps.end());
    }
  }

  if (prices_.size() != (old_size + ps.size() - 1))
    throw std::runtime_error("Prices size mismatch after adding more data");

  // now try to find the requested price again, if we don't have it this time,
  // then not enough data was provided
  idx = day - start_day_;

  if ((idx >= 0) && (idx < (int64_t)prices_.size())) {
    return prices_[idx];
  } else if (idx == (int64_t)prices_.size()) {
    printf(
        "WARNING: Price for current day is not available yet, using price from "
        "yesterday\n");
    return prices_[idx - 1];
  } else {
    printf(
        "WARNING: Couldn't get requested price after fetching more data, "
        "return latest price\n");
    return prices_[0];
  }
}

std::pair<std::vector<int64_t>, std::vector<Amount>> DailyData::GetData(
    int64_t from, int64_t to) const {
  if (coin_->NumId() <= 0)
    throw std::invalid_argument("Can't get daily data for " + coin_->Id() +
                                " because it doesn't have a num_id");

  if (to < from) throw std::invalid_argument("to must be larger than from");
  std::string url =
      "https://web-api.coinmarketcap.com/v1/cryptocurrency/ohlcv/historical?";
  url += "id=" + std::to_string(coin_->NumId());
  url +=
      "&convert=USD&time_start=" + std::to_string(from * 24 * 3600 - 12 * 3600);
  url += "&time_end=" + std::to_string(to * 24 * 3600 + 12 * 3600);

  printf("fetching %s\n", url.c_str());
  auto json = PriceSource::GetURL(url);
  std::stringstream ss;
  ss.str(json);

  try {
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    Json::Value root;
    std::string parse_errors;

    if (!Json::parseFromStream(rbuilder, ss, &root, &parse_errors))
      throw std::runtime_error("Could not parse __NEXT_DATA__ JSON from " +
                               url + ": " + parse_errors);
    auto data = root["data"];
    if (data["symbol"].asString() != coin_->Symbol())
      throw std::runtime_error(
          "Historic data symbol mismatch: " + data["symbol"].asString() +
          " != " + coin_->Symbol());

    // read the table
    std::vector<int64_t> days;
    std::vector<Amount> prices;
    {
      for (const auto& q : data["quotes"]) {
        std::string time_str = q["quote"]["USD"]["timestamp"].asString();
        std::string close_str = q["quote"]["USD"]["close"].asString();

        auto price = Amount::Parse(close_str);

        auto day = Datetime::DailyDataDayFromStr(time_str);
        if ((days.size() > 0) && (day != (days[days.size() - 1] - 1))) {
          // fill in gap
          for (int64_t d = days[days.size() - 1] - 1; d > day; --d) {
            days.push_back(d);
            prices.push_back(price);
          }
        }
        days.push_back(day);
        prices.push_back(price);
      }
    }

    return {days, prices};
  } catch (const std::exception& ex) {
    printf("ERROR while parsing historical prices: %s\n", ex.what());
    throw ex;
  }
}