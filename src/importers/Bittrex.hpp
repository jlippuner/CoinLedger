/// \file Bittrex.hpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_BITTREX_HPP_
#define SRC_IMPORTERS_BITTREX_HPP_

#include <memory>

#include "Account.hpp"
#include "File.hpp"

class Bittrex {
 public:
  static void Import(const std::string& import_file, File* file,
      std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account);
};

#endif  // SRC_IMPORTERS_BITTREX_HPP_