/// \file Bittrex.cpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#include "Bittrex.hpp"

#include <regex>

#include "CSV.hpp"

void Bittrex::Import(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"OrderUuid", "Exchange", "Type",
      "Quantity", "Limit", "CommissionPaid", "Price", "Opened", "Closed"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto uuid = rec[0];
    auto exch = rec[1];
    auto type = rec[2];
    auto quote_amt = Amount::Parse(rec[3]);
    auto fee = Amount::Parse(rec[5]);
    auto base_amt = Amount::Parse(rec[6]);
    auto time = Datetime::FromBittrex(rec[8]);

    std::string tx_id = "Bittrex_" + uuid;
    std::string tx_description = "Bittrex trade " + exch;

    if (type == "LIMIT_SELL") {
      quote_amt = -quote_amt;
    } else if (type == "LIMIT_BUY") {
      base_amt = -base_amt;
    } else {
      throw std::runtime_error("Unknown Bittrex type '" + type + "'");
    }

    // get quote and base currencies
    std::regex reg(R"(^([A-Z]+)-([A-Z]+)$)");
    std::smatch m;
    if (!std::regex_match(exch, m, reg))
      throw std::invalid_argument(
          "Can't parse Bittrex exchange '" + exch + "'");

    auto base = file->GetCoinBySymbol(m[1].str());
    auto quote = file->GetCoinBySymbol(m[2].str());

    if ((base == nullptr) || (quote == nullptr))
      throw std::invalid_argument(
          "Unknown coins in Bittrex exchange '" + exch + "'");

    // check if a transaction with this import_id already exists
    if (file->TransactionsByImportId().count(tx_id) > 0) {
      // transaction already exists, skip this since the Bittrex file contains
      // one transaction at a time
      ++num_duplicate;
      continue;
    }

    // we will create 4 splits
    std::vector<ProtoSplit> splits;

    // main split pair of the trade
    splits.push_back(ProtoSplit(account, "", base_amt, base, ""));
    splits.push_back(ProtoSplit(account, "", quote_amt, quote, ""));

    // the fee split pair
    splits.push_back(ProtoSplit(account, "", -fee, base, ""));
    splits.push_back(ProtoSplit(fee_account, "", fee, base, ""));

    Transaction::Create(file, time, tx_description, splits, tx_id);
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}