#ifndef SRC_PRICES_DAILYDATA_HPP_
#define SRC_PRICES_DAILYDATA_HPP_

#include <memory>
#include <vector>

#include "Amount.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"

class DailyData {
 public:
  DailyData(std::shared_ptr<const Coin> coin) : coin_(coin), start_day_(0) {}
  DailyData(std::shared_ptr<const Coin> coin, int64_t start_day,
      const std::vector<Amount>& prices)
      : coin_(coin), start_day_(start_day), prices_(prices) {}

  std::shared_ptr<const Coin> GetCoin() const { return coin_; }
  int64_t StartDay() const { return start_day_; }
  const std::vector<Amount>& Prices() const { return prices_; }

  Amount operator()(const Datetime& date);

 private:
  std::pair<std::vector<int64_t>, std::vector<Amount>> GetData(
      int64_t from, int64_t to) const;

  // coin whose prices are stored
  std::shared_ptr<const Coin> coin_;

  // days are given as (UNIX timestamp in UTC) / 86400, the start_day_ is the
  // day corresponding to prices_[0] and prices_ contains the price of all
  // subsequent days without gaps
  int64_t start_day_;

  // daily closing prices in USD
  std::vector<Amount> prices_;
};

#endif  // SRC_PRICES_DAILYDATA_HPP_