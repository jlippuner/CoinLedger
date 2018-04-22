/// \file MiningPoolHub.hpp
/// \author jlippuner
/// \since Apr 22, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_MININGPOOLHUB_HPP_
#define SRC_IMPORTERS_MININGPOOLHUB_HPP_

#include <memory>

#include "Account.hpp"
#include "File.hpp"

class MiningPoolHub {
 public:
  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<const Coin> coin, std::shared_ptr<Account> pool_account,
      std::shared_ptr<Account> mining_income_account,
      std::shared_ptr<Account> mining_fee_account,
      std::shared_ptr<Account> transaction_fee_account);
};

#endif  // SRC_IMPORTERS_MININGPOOLHUB_HPP_