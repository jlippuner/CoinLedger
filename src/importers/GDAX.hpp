/// \file GDAX.hpp
/// \author jlippuner
/// \since Apr 05, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_GDAX_HPP_
#define SRC_IMPORTERS_GDAX_HPP_

#include <memory>

#include "Account.hpp"
#include "File.hpp"

class GDAX {
public:
  static void Import(const std::string& import_file, File * file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
      std::shared_ptr<Account> usd_investment_account);
};

#endif // SRC_IMPORTERS_GDAX_HPP_