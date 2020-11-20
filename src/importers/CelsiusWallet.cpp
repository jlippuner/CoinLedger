/// \file CelsiusWallet.cpp
/// \author jlippuner
/// \since Nov 20, 2020
///
/// \brief
///
///

#include "CelsiusWallet.hpp"

#include "CSV.hpp"
#include "prices/PriceSource.hpp"

void CelsiusWallet::Import(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> /*fee_account*/,
    std::shared_ptr<Account> interest_account,
    std::shared_ptr<Account> referral_award_account,
    const std::map<std::string, std::string>& transaction_associations) {
  if (account->SingleCoin()) {
    throw std::invalid_argument(
        "Celsius wallets can only be imported into a multi-coin account");
  }
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"Internal id", "Date and time",
      "Transaction type", "Coin type", "Coin amount", "USD Value",
      "Original Interest Coin", "Interest Amount In Original Coin",
      "Confirmed"};

  if (csv.Header() != expected_header) {
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");
  }

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto id = rec[0];
    auto time = Datetime::FromCelsius(rec[1]);
    auto type = rec[2];
    auto coin_str = rec[3];
    auto coin = file->GetCoinBySymbol(coin_str);
    auto amount = Amount::Parse(rec[4]);

    std::string tx_id = "Celsius_" + id;
    if (transaction_associations.count(id) > 0) {
      tx_id = transaction_associations.at(id);
    }

    std::string split_id = "Celsius_" + coin_str + "_" + type + "_" + id;
    std::string tx_description = "Celsius " + coin_str + " " + type;

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

    // main split in the Celsius Wallet account
    splits.push_back(ProtoSplit(account, "", amount, coin, split_id));

    if (type == "deposit") {
      // nothing to do
    } else if (type == "interest") {
      splits.push_back(
          ProtoSplit(interest_account, "", -amount, coin, split_id));
    } else if (type == "referred_award") {
      splits.push_back(
          ProtoSplit(referral_award_account, "", -amount, coin, split_id));
    } else {
      throw std::runtime_error("Unknown Celsius transaction type " + type);
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
      txn->SetDate(time);
    }
    ++num_imported;
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}