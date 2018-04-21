/// \file XRPAccount.hpp
/// \author jlippuner
/// \since Apr 20, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_XRPACCOUNT_HPP_
#define SRC_IMPORTERS_XRPACCOUNT_HPP_

#include <map>
#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class XRPAccount {
 public:
  static void Import(const std::string& xrp_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      const std::map<std::string, std::string>& transaction_associations);

  static void Import(const std::string& xrp_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
    Import(xrp_account, file, account, fee_account,
        std::map<std::string, std::string>());
  }
};

#endif  // SRC_IMPORTERS_XRPACCOUNT_HPP_