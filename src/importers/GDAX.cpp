/// \file GDAX.cpp
/// \author jlippuner
/// \since Apr 05, 2018
///
/// \brief
///
///

#include "GDAX.hpp"

#include "CSV.hpp"

void GDAX::Import(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
    std::shared_ptr<Account> usd_investment_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"type", "time", "amount", "balance",
      "amount/balance unit", "transfer id", "trade id", "order id"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto type = rec[0];
    auto time = Datetime::FromISO8601(rec[1]);
    auto amount = Amount::Parse(rec[2]);
    auto coin_str = rec[4];
    auto coin = file->GetCoinBySymbol(coin_str);
    auto transfer_id = rec[5];
    auto trade_id = rec[6];

    std::string tx_id = "GDAX_";
    std::string split_id = tx_id + coin_str + "_" + type + "_";
    std::string id_suffix = "";
    std::string tx_description = "GDAX ";

    if ((type == "deposit") || (type == "withdrawal")) {
      id_suffix = transfer_id;
      tx_description += coin_str + " " + type;
    } else if ((type == "match") || (type == "fee")) {
      id_suffix = trade_id;
      tx_description += "trade";
    } else {
      throw std::runtime_error("Unknown GDAX type '" + type + "'");
    }

    tx_id += id_suffix;
    split_id += id_suffix;

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // if a transaction with this import id already exists, check if it has a
    // split with this split id, in which case we don't need to duplicate this
    // split
    if ((txn != nullptr) && (txn->HasSplitWithImportId(split_id))) {
      ++num_duplicate;
      continue;
    }

    // we will create one or two splits, depending on the type of record
    std::vector<ProtoSplit> splits;

    // main split in the GDAX account
    splits.push_back(ProtoSplit(account, "", amount, coin, split_id));

    if (type == "fee") {
      // create split in the trading fee account
      splits.push_back(ProtoSplit(fee_account, "", -amount, coin, split_id));
    }
    if ((coin_str == "USD") &&
        ((type == "deposit") || (type == "withdrawal"))) {
      // if this is a USD deposit or withdrawal, we can add the matching split
      // in the USD investment account
      splits.push_back(
          ProtoSplit(usd_investment_account, "", -amount, coin, split_id));
    }

    if (txn == nullptr) {
      // we need to create a new transaction
      Transaction::Create(file, time, tx_description, splits, tx_id);
    } else {
      // the transaction already exists, just add the new splits
      for (auto& proto_s : splits) {
        auto split = Split::Create(file, txn, proto_s);
        txn->AddSplit(split);
      }
    }
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}