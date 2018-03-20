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

// Represents a cryptocurrency or a fiat currency

class Coin {
private:
  // unique global identifier of this coin
  const boost::uuids::uuid id_;

  // name of this coin
  std::string name_;

  // symbol of this coin
  std::string symbol_;
};

#endif // SRC_COIN_HPP_
