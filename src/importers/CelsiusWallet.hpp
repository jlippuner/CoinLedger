/// \file CelsiusWallet.hpp
/// \author jlippuner
/// \since Nov 20, 2020
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_CELSIUSWALLET_HPP_
#define SRC_IMPORTERS_CELSIUSWALLET_HPP_

#include <map>
#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class CelsiusWallet {
 public:
  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> interest_account,
      std::shared_ptr<Account> referral_award_account,
      const std::map<std::string, std::string>& transaction_associations);

  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> interest_account,
      std::shared_ptr<Account> referral_award_account) {
    Import(import_file, file, account, fee_account, interest_account,
        referral_award_account, std::map<std::string, std::string>());
  }
};

#endif  // SRC_IMPORTERS_CELSIUSWALLET_HPP_