/// \file Coin.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Coin.hpp"

#include "File.hpp"

std::shared_ptr<Coin> Coin::Create(
    File* file, std::string id, std::string name, std::string symbol) {
  if (file->Coins().count(id) > 0) {
    return file->GetCoin(id);
  } else {
    return file->AddCoin(Coin(id, name, symbol));
  }
}
