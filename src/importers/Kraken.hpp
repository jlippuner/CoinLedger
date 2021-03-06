/// \file Kraken.hpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_KRAKEN_HPP_
#define SRC_IMPORTERS_KRAKEN_HPP_

#include <memory>

#include "Account.hpp"
#include "File.hpp"

class Kraken {
 public:
  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account,
      std::shared_ptr<Account> trading_fee_account,
      std::shared_ptr<Account> transaction_fee_account,
      std::shared_ptr<Account> usd_investment_account) {
    Import(import_file, file, account, trading_fee_account,
        transaction_fee_account, usd_investment_account,
        std::map<std::string, std::string>());
  }

  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account,
      std::shared_ptr<Account> trading_fee_account,
      std::shared_ptr<Account> transaction_fee_account,
      std::shared_ptr<Account> usd_investment_account,
      const std::map<std::string, std::string>& transaction_associations);
};

#endif  // SRC_IMPORTERS_KRAKEN_HPP_