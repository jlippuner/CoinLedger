/// \file ETHAccount.hpp
/// \author jlippuner
/// \since Apr 21, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_ETHACCOUNT_HPP_
#define SRC_IMPORTERS_ETHACCOUNT_HPP_

#include <map>
#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class ETHAccount {
 public:
  static void Import(const std::string& eth_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      const std::map<std::string, std::string>& transaction_associations);

  static void Import(const std::string& eth_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
    Import(eth_account, file, account, fee_account,
        std::map<std::string, std::string>());
  }
};

#endif  // SRC_IMPORTERS_ETHACCOUNT_HPP_