/// \file NiceHash.cpp
/// \author jlippuner
/// \since Apr 22, 2018
///
/// \brief
///
///

#include "NiceHash.hpp"

#include "CSV.hpp"
#include "prices/PriceSource.hpp"

void NiceHash::ImportTransactions(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> mining_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{
      "Date", "Comment", "Amount", "Currency", "BTC/USD"};

  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto time = Datetime::FromNiceHashLocal(rec[0]);
    auto label = rec[1];
    auto amount = Amount::Parse(rec[2]);
    auto coin_str = rec[3];
    auto coin = file->GetCoinBySymbol(coin_str);

    std::string tx_id = "NiceHash_" + coin->Symbol() + "_" + label;
    std::string tx_description = label;

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // we always create both splits at once
    if (txn != nullptr) {
      ++num_duplicate;
      continue;
    }

    ProtoSplit sp1(account, "", amount, coin, tx_id + "_pool");
    ProtoSplit sp2(mining_account, "", -amount, coin, tx_id + "_income");
    Transaction::Create(file, time, tx_description, {sp1, sp2}, tx_id);
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}

void NiceHash::ImportWithdrawals(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"Date", "Address", "Amount",
      "Currency", "Fee", "Currency", "TXID", "BTC/USD"};

  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto time = Datetime::FromNiceHashLocal(rec[0]);
    auto amount = Amount::Parse(rec[2]);
    auto coin_str = rec[3];
    auto coin = file->GetCoinBySymbol(coin_str);
    auto fee_amount = Amount::Parse(rec[4]);
    auto fee_coin_str = rec[5];
    auto txid = rec[6];

    if (fee_amount < 0)
      throw std::invalid_argument(
          "Fee cannot be negative in NiceHash withdrawals");

    if (fee_coin_str != coin_str)
      throw std::runtime_error(
          "Fee coin different from withdrawal coin not implemented in NiceHash "
          "withdrawals");

    std::string tx_id = coin_str + "_" + txid;
    std::string tx_description = "Nicehash " + coin_str + " withdrawal";

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // we always create both splits at once
    if (txn != nullptr) {
      ++num_duplicate;
      continue;
    }

    std::vector<ProtoSplit> splits;

    splits.push_back(
        ProtoSplit(account, "", -(amount + fee_amount), coin, tx_id + "_pool"));

    if (fee_amount > 0)
      splits.push_back(
          ProtoSplit(fee_account, "", amount, coin, tx_id + "_fee"));

    Transaction::Create(file, time, tx_description, splits, tx_id);
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}