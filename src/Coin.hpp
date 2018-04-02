/// \file Coin.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_COIN_HPP_
#define SRC_COIN_HPP_

#include <string>

#include <boost/uuid/uuid.hpp>

class File;

// Represents a cryptocurrency or a fiat currency

class Coin {
public:
  static Coin * Create(File* file, std::string id, std::string name,
      std::string symbol);

  std::string Id() const { return id_; }
  const std::string& Name() const { return name_; }
  const std::string& Symbol() const { return symbol_; }

private:
  friend class File;

  Coin(std::string id, std::string name, std::string symbol):
    id_(id),
    name_(name),
    symbol_(symbol) {}

  // unique global identifier of this coin
  const std::string id_;

  // name of this coin
  std::string name_;

  // symbol of this coin
  std::string symbol_;
};

#endif // SRC_COIN_HPP_
