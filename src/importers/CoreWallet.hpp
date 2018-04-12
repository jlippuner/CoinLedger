/// \file CoreWallet.hpp
/// \author jlippuner
/// \since Apr 11, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_COREWALLET_HPP_
#define SRC_IMPORTERS_COREWALLET_HPP_

#include <map>
#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class CoreWallet {
 public:
  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> mining_account,
      const std::vector<std::string>& mining_labels,
      const std::map<std::string, std::string>& transaction_associations);

  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> mining_account,
      const std::vector<std::string>& mining_labels) {
    Import(import_file, file, account, fee_account, mining_account,
        mining_labels, std::map<std::string, std::string>());
  }

  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> mining_account,
      const std::map<std::string, std::string>& transaction_associations) {
    Import(import_file, file, account, fee_account, mining_account, {},
        transaction_associations);
  }

  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> mining_account) {
    Import(import_file, file, account, fee_account, mining_account, {},
        std::map<std::string, std::string>());
  }
};

#endif  // SRC_IMPORTERS_COREWALLET_HPP_