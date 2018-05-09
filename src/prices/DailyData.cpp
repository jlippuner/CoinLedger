#include "DailyData.hpp"

#include <htmlcxx/html/ParserDom.h>

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

  auto data = GetData(from, to);
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
    if (ps[ps.size() - 1] != prices_[0])
      throw std::runtime_error("Price mismatch between new and existing data");

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
      if (ps[0] != prices_[prices_.size() - 1])
        throw std::runtime_error(
            "Price mismatch between new and existing data");

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
  } else {
    throw std::runtime_error(
        "Couldn't get requested price after fetching more data");
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

    // find div with id historical-data
    auto itr = tree.begin();
    bool found = false;
    for (; itr != tree.end(); ++itr) {
      if (itr->tagName() == "div") {
        itr->parseAttributes();
        auto id = itr->attribute("id");
        if (id.first && (id.second == "historical-data")) {
          found = true;
          break;
        }
      }
    }
    if (!found) throw std::runtime_error("Could not find historical-data");

    auto st = tree.subtree(tree.begin(itr), tree.end(itr));

    // find thead and tbody
    auto thead = st.begin();
    found = false;
    for (; thead != st.end(); ++thead) {
      if (thead->tagName() == "thead") {
        found = true;
        break;
      }
    }
    if (!found) throw std::runtime_error("Couldn't find thead");
    auto tbody = st.begin();
    found = false;
    for (; tbody != st.end(); ++tbody) {
      if (tbody->tagName() == "tbody") {
        found = true;
        break;
      }
    }
    if (!found) throw std::runtime_error("Couldn't find tbody");

    // check the header
    {
      std::vector<std::string> header;

      for (auto itr = st.begin(thead); itr != st.end(thead); ++itr) {
        if (itr->tagName() == "tr") {
          for (auto th = st.begin(itr); th != st.end(itr); ++th) {
            if (th->tagName() == "th") {
              for (auto c = st.begin(th); c != st.end(th); ++c)
                header.push_back(c->text());
            }
          }
        }
      }

      if (header != std::vector<std::string>{"Date", "Open", "High", "Low",
                        "Close", "Volume", "Market Cap"})
        throw std::runtime_error("Unexpected header");
    }

    // read the table
    std::vector<int64_t> days;
    std::vector<Amount> prices;
    {
      for (auto itr = st.begin(tbody); itr != st.end(tbody); ++itr) {
        if (itr->tagName() == "tr") {
          std::vector<std::string> row;

          for (auto td = st.begin(itr); td != st.end(itr); ++td) {
            if (td->tagName() == "td") {
              for (auto c = st.begin(td); c != st.end(td); ++c)
                row.push_back(c->text());
            }
          }

          if (row.size() != 7)
            throw std::runtime_error("Could not parse table row");

          auto day = Datetime::DailyDataDayFromStr(row[0]);
          if ((days.size() > 0) && (day != (days[days.size() - 1] - 1))) {
            // fill in gap
            for (int64_t d = days[days.size() - 1] - 1; d > day; --d) {
              days.push_back(d);
              prices.push_back(Amount::Parse(row[4]));
            }
          }
          days.push_back(day);
          prices.push_back(Amount::Parse(row[4]));
        }
      }
    }

    // the data is provided in reverse order
    std::reverse(days.begin(), days.end());
    std::reverse(prices.begin(), prices.end());

    return {days, prices};
  } catch (const std::exception& ex) {
    printf("ERROR while parsing historical prices: %s\n", ex.what());
    throw ex;
  }
}