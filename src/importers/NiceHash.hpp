/// \file NiceHash.hpp
/// \author jlippuner
/// \since Apr 22, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_NICEHASH_HPP_
#define SRC_IMPORTERS_NICEHASH_HPP_

#include <memory>
#include <string>

#include "Account.hpp"
#include "File.hpp"

class NiceHash {
 public:
  static void ImportTransactions(const std::string& import_file, File* file,
      std::shared_ptr<Account> account,
      std::shared_ptr<Account> mining_account);
};

#endif  // SRC_IMPORTERS_NICEHASH_HPP_