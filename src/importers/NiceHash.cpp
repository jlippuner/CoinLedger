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