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
  if (to < from) throw std::invalid_argument("to must be larger than from");

  auto from_str = Datetime::ToStrDailyData(from);
  auto to_str = Datetime::ToStrDailyData(to);
  // https://coinmarketcap.com/currencies/verge/historical-data/?start=20170701&end=20170802
  std::string url = "https://coinmarketcap.com/currencies/" + coin_->Id() +
                    "/historical-data/?start=" + from_str + "&end=" + to_str;

  printf("fetching %s\n", url.c_str());
  auto res = PriceSource::GetURL(url);

  try {
    using namespace htmlcxx::HTML;
    ParserDom parser;
    auto tree = parser.parseTree(res);

    // get the content of <script id="__NEXT_DATA__">
    auto itr = tree.begin();
    bool found = false;
    for (; itr != tree.end(); ++itr) {
      if (itr->tagName() == "script") {
        itr->parseAttributes();
        auto id = itr->attribute("id");
        if (id.first && (id.second == "__NEXT_DATA__")) {
          found = true;
          break;
        }
      }
    }
    if (!found) throw std::runtime_error("Could not find __NEXT_DATA__");

    auto json = tree.begin(itr)->text();
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(json, root))
      throw std::runtime_error(
          "Could not parse __NEXT_DATA__ JSON from " + url);

    auto hist_root =
        root["props"]["initialState"]["cryptocurrency"]["ohlcvHistorical"];

    if (hist_root.size() != 1)
      throw std::runtime_error("Expected just one element but got " +
                               std::to_string(hist_root.size()));

    auto hist = *hist_root.begin();
    if (hist["symbol"].asString() != coin_->Symbol())
      throw std::runtime_error(
          "Historic data symbol mismatch: " + hist["symbol"].asString() +
          " != " + coin_->Symbol());

    // read the table
    std::vector<int64_t> days;
    std::vector<Amount> prices;
    {
      for (const auto& q : hist["quotes"]) {
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