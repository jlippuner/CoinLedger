/// \file File.cpp
/// \author jlippuner
/// \since Mar 21, 2018
///
/// \brief
///
///

#include "File.hpp"

#include "PriceSource.hpp"

File File::InitNewFile() {
  File file;

  // create root accounts
  Account::Create(&file, "Assets",      true, nullptr, false);
  Account::Create(&file, "Liabilities", true, nullptr, false);
  Account::Create(&file, "Income",      true, nullptr, false);
  Account::Create(&file, "Expenses",    true, nullptr, false);
  Account::Create(&file, "Equity",      true, nullptr, false);

  // add all known coins
  PriceSource::AddAllCoins(&file);

  return file;
}
