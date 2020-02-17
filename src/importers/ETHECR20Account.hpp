/// \file ETHECR20Account.hpp
/// \author jlippuner
/// \since Feb 16, 2020
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_ETHECR20ACCOUNT_HPP_
#define SRC_IMPORTERS_ETHECR20ACCOUNT_HPP_

#include <map>
#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class ETHECR20Account {
 public:
  static void Import(const std::string& eth_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> eth_account_for_fee,
      const std::map<std::string, std::string>& transaction_associations);

  static void Import(const std::string& eth_account, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> eth_account_for_fee) {
    Import(eth_account, file, account, fee_account, eth_account_for_fee,
        std::map<std::string, std::string>());
  }
};

#endif  // SRC_IMPORTERS_ETHECR20ACCOUNT_HPP_