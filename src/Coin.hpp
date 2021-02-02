/// \file Coin.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_COIN_HPP_
#define SRC_COIN_HPP_

#include <memory>
#include <string>

class File;

// Represents a cryptocurrency or a fiat currency

class Coin {
 public:
  static std::string USD_id() { return "us-dollar"; }

  static std::shared_ptr<Coin> Create(File* file, std::string id,
      std::string name, std::string symbol, int num_id);

  std::string Id() const { return id_; }
  const std::string& Name() const { return name_; }
  const std::string& Symbol() const { return symbol_; }
  int NumId() const { return num_id_; }

  bool IsUSD() const { return id_ == USD_id(); }

  void SetNumId(int num_id) { num_id_ = num_id; }

 private:
  friend class File;

  Coin(std::string id, std::string name, std::string symbol, int num_id)
      : id_(id), name_(name), symbol_(symbol), num_id_(num_id) {}

  // unique global identifier of this coin
  const std::string id_;

  // name of this coin
  std::string name_;

  // symbol of this coin
  std::string symbol_;

  // CoinMarketCap numeric id
  int num_id_;
};

#endif  // SRC_COIN_HPP_
