/// \file Binance.cpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#include "Binance.hpp"

#include <regex>

#include "CSV.hpp"

void Binance::ImportTrades(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"Date(UTC)", "Market", "Type",
      "Price", "Amount", "Total", "Fee", "Fee Coin"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto time = Datetime::FromUTC(rec[0]);
    auto market = rec[1];
    auto type = rec[2];
    auto quote_amt = Amount::Parse(rec[4]);
    auto base_amt = Amount::Parse(rec[5]);
    auto fee = Amount::Parse(rec[6]);
    auto fee_coin_str = rec[7];

    // Binance does not provide a unique identifier, just store the entire line
    // and hope there are no completely identical trades
    std::string tx_id = "Binance_" + time.ToStrLocalFile() + "_" + rec[1] +
                        "_" + rec[2] + "_" + rec[3] + "_" + rec[4] + "_" +
                        rec[5] + "_" + rec[6] + "_" + rec[7];
    std::string tx_description = "Binance trade " + market;

    if (type == "SELL") {
      quote_amt = -quote_amt;
    } else if (type == "BUY") {
      base_amt = -base_amt;
    } else {
      throw std::runtime_error("Unknown Binance type '" + type + "'");
    }

    // get quote and base currencies, valid bases are BTC, ETH, BNB, USDT
    std::regex reg(R"(^([0-9A-Z]+)(BTC|ETH|BNB|USDT)$)");
    std::smatch m;
    if (!std::regex_match(market, m, reg))
      throw std::invalid_argument(
          "Can't parse Binance market '" + market + "'");

    auto quote = file->GetCoinBySymbol(m[1].str());
    auto base = file->GetCoinBySymbol(m[2].str());

    if ((base == nullptr) || (quote == nullptr))
      throw std::invalid_argument(
          "Unknown coins in Binance exchange '" + market + "'");

    auto fee_coin = file->GetCoinBySymbol(fee_coin_str);
    if (fee_coin == nullptr)
      throw std::invalid_argument(
          "Unknown Binance fee coin '" + fee_coin_str + "'");

    // check if a transaction with this import_id already exists
    if (file->TransactionsByImportId().count(tx_id) > 0) {
      // transaction already exists, skip this since the Binance file contains
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
    splits.push_back(ProtoSplit(account, "", -fee, fee_coin, ""));
    splits.push_back(ProtoSplit(fee_account, "", fee, fee_coin, ""));

    Transaction::Create(file, time, tx_description, splits, tx_id);
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}

void Binance::ImportDeposits(const std::string& import_file, File* file,
    std::shared_ptr<Account> account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"Date", "Coin", "Amount",
      "TransactionFee", "Address", "TXID", "SourceAddress", "PaymentID",
      "Status"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto time = Datetime::FromUTC(rec[0]);
    auto coin_str = rec[1];
    auto coin = file->GetCoinBySymbol(coin_str);
    auto amount = Amount::Parse(rec[2]);
    auto txfee = Amount::Parse(rec[3]);
    auto txid = rec[5];
    auto status = rec[8];

    if (txfee != 0)
      throw std::invalid_argument(
          "Cannot handle non-zero Binance deposit transaction fee");

    if (status != "Completed") continue;

    if (coin == nullptr)
      throw std::invalid_argument("Unknown coin '" + coin_str + "'");

    std::string tx_id = coin_str + "_" + txid;
    std::string split_id = "Binance_deposit_" + tx_id;
    std::string tx_description = "Binance " + coin_str + " deposit";

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // if a transaction with this import id already exists, check if it has a
    // split with this split id, in which case we don't need to duplicate this
    // split
    if ((txn != nullptr) && (txn->HasSplitWithImportId(split_id))) {
      ++num_duplicate;
      continue;
    }

    // create a new split for this deposit
    ProtoSplit proto_split(account, "", amount, coin, split_id);

    if (txn == nullptr) {
      // we need to create a new transaction
      Transaction::Create(file, time, tx_description, {proto_split}, tx_id);
    } else {
      // the transaction already exists, just add the new split
      auto split = Split::Create(file, txn, proto_split);
      txn->AddSplit(split);
    }
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}
