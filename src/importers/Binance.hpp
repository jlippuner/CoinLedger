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
  static void Import(const std::string& import_file, File* file) {
    Import(import_file, file, std::map<std::string, std::string>());
  }

  static void Import(const std::string& import_file, File* file,
      const std::map<std::string, std::string>& transaction_associations);
};

#endif  // SRC_IMPORTERS_BINANCE_HPP_