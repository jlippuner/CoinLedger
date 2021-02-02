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
#include "Split.hpp"

void Binance::Import(const std::string& import_file, File* file,
    const std::map<std::string, std::string>& transaction_associations) {
  // get accounts
  auto earn = file->GetAccount("Assets::Exchanges::Binance::Earn");
  auto trade_fee = file->GetAccount("Expenses::Trading Fees::Binance");
  auto txn_fee =
      file->GetAccount("Expenses::Transaction Fees::Binance Withdrawals");
  auto interest = file->GetAccount("Income::Other Income::Interest::Binance");
  auto pnl = file->GetAccount("Income::Trading P/L::Binance P/L");
  auto airdrop = file->GetAccount("Income::Other Income::Airdrop");
  auto loan = file->GetAccount("Liabilities::Binance Loan");

  std::map<std::string, std::shared_ptr<Account>> accnts;
  accnts.insert({"Spot", file->GetAccount("Assets::Exchanges::Binance::Spot")});
  accnts.insert({"USDT-Futures",
      file->GetAccount("Assets::Exchanges::Binance::USDT Futures")});

  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{
      "UTC_Time", "Account", "Operation", "Coin", "Change", "Remark"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_possible_duplicate = 0;
  size_t num_imported = 0;

  std::string last_accnt = "";
  bool last_was_loan = false;

  for (auto& rec : csv.Content()) {
    auto time_str = rec[0];
    auto time = Datetime::FromUTC(time_str);
    auto accnt = rec[1];
    auto op = rec[2];
    auto coin_str = rec[3];
    auto coin = file->GetCoinBySymbol(coin_str);
    auto change = Amount::Parse(rec[4]);

    bool is_loan = (op == "Cross Collateral Transfer");

    // Binance does not provice transaction IDs, so we use the timestamp to
    // construct the ID, which allows us to correctly match splits that belong
    // to the same transaction
    std::regex reg(R"([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})");
    if (!std::regex_match(time_str, reg))
      throw std::invalid_argument(time_str + " is not a valid datetime");

    time_str[10] = '_';
    std::string tx_id = "Binance_" + time_str;

    if (transaction_associations.count(tx_id) > 0) {
      tx_id = transaction_associations.at(tx_id);
    }

    std::string split_id =
        tx_id + "_" + accnt + "_" + op + "_" + coin_str + "_" + rec[4];

    std::string tx_description = "Binance " + coin_str + " " + op;
    if ((accnt == "Spot") && ((op == "Buy") || (op == "Sell") || (op == "Fee")))
      tx_description = "Binance Spot trade";

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // if a transaction with this import id already exists, check if it has a
    // split with this split id
    if ((txn != nullptr) && (txn->HasSplitWithImportId(split_id))) {
      // it is possible that we legitimately get the same split, so just warn
      // here
      printf("WARNING: Possible duplicate: %s\n", split_id.c_str());
      ++num_possible_duplicate;
    }

    // we will create one or two splits, depending on the type of record
    std::vector<ProtoSplit> splits;
    auto account = accnts.at(accnt);

    // create a new split(s) for this deposit
    splits.push_back(ProtoSplit(account, "", change, coin, split_id));

    if ((op == "Buy") || (op == "Sell") || (op == "Deposit") ||
        (op == "transfer_in") || (op == "transfer_out") ||
        (op == "Large OTC trading")) {
      // nothing to do
    } else if (op == "Fee") {
      splits.push_back(
          ProtoSplit(trade_fee, "", -change, coin, split_id + "_fee"));
    } else if (op == "Realize profit and loss") {
      if (op == "Realize profit and loss")
        splits.push_back(ProtoSplit(pnl, "", -change, coin, split_id));
    } else if ((op == "Savings purchase") ||
               (op == "Savings Principal redemption")) {
      splits.push_back(ProtoSplit(earn, "", -change, coin, split_id + "_earn"));
    } else if (op == "Savings Interest") {
      splits.push_back(
          ProtoSplit(interest, "", -change, coin, split_id + "_interest"));
    } else if (op == "Savings Interest MANUAL ADJUSTMENT") {
      splits.clear();
      splits.push_back(ProtoSplit(earn, "", change, coin, split_id));
      splits.push_back(
          ProtoSplit(interest, "", -change, coin, split_id + "_interest"));
    } else if (op == "Distribution") {
      // this could be airdrop or move to futures collateral
      if (last_was_loan) {
        // assume this is collateral
        splits.push_back(ProtoSplit(accnts.at(last_accnt), "", -change, coin,
            split_id + "_collateral"));
      } else {
        // assume airdrop
        splits.push_back(ProtoSplit(airdrop, "", -change, coin, split_id));
      }
    } else if (op == "Cross Collateral Transfer") {
      splits.push_back(ProtoSplit(loan, "", -change, coin, split_id + "_loan"));
    } else {
      throw std::invalid_argument("Unknown operation " + op);
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
    last_was_loan = is_loan;
    last_accnt = accnt;
  }

  printf("Imported %lu records and got %lu POSSIBLE duplicates from %s\n",
      num_imported, num_possible_duplicate, import_file.c_str());
}
