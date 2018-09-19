/// \file Binance.hpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_BINANCE_HPP_
#define SRC_IMPORTERS_BINANCE_HPP_

#include <memory>

#include "Account.hpp"
#include "File.hpp"

class Binance {
 public:
  static void ImportTrades(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account);

  static void ImportDeposits(const std::string& import_file, File* file,
      std::shared_ptr<Account> account) {
    ImportDepositsOrWithdrawals(true, import_file, file, account, nullptr);
  }

  static void ImportWithdrawals(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
    ImportDepositsOrWithdrawals(false, import_file, file, account, fee_account);
  }

 private:
  static void ImportDepositsOrWithdrawals(bool deposits,
      const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account);
};

#endif  // SRC_IMPORTERS_BINANCE_HPP_