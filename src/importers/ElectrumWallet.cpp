/// \file ElectrumWallet.cpp
/// \author jlippuner
/// \since Apr 20, 2018
///
/// \brief
///
///

#include "ElectrumWallet.hpp"

#include "CSV.hpp"
#include "prices/PriceSource.hpp"

void ElectrumWallet::Import(const std::string& import_file, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
    std::shared_ptr<Account> mining_account,
    const std::vector<std::string>& mining_labels,
    const std::map<std::string, std::string>& transaction_associations) {
  if (!account->SingleCoin()) {
    throw std::invalid_argument(
        "Electrum wallets can only be imported into a single-coin account");
  }
  auto coin = account->GetCoin();

  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{
      "transaction_hash", "label", "confirmations", "value", "timestamp"};

  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& rec : csv.Content()) {
    auto id = rec[0];
    auto label = rec[1];
    auto confirmations = std::stoi(rec[2]);
    auto amount = Amount::Parse(rec[3]);
    auto time = Datetime::FromElectrumLocal(rec[4]);

    if (confirmations < 6) continue;

    std::string tx_id = coin->Symbol() + "_" + id;
    if (transaction_associations.count(id) > 0) {
      tx_id = transaction_associations.at(id);
    }

    std::string split_id = coin->Symbol() + "_Electrum_" + id;
    std::string tx_description = label;

    // check if the label is in the list of mining labels
    bool mined = false;
    for (auto& l : mining_labels) {
      if (l == label) {
        mined = true;
        break;
      }
    }

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

    // main split in the Electrum Wallet account
    splits.push_back(ProtoSplit(account, "", amount, coin, split_id));

    // if this split sends out money, try to figure out the fee
    if (amount < 0) {
      Amount fee = 0;
      if (txn != nullptr) {
        // we have an associated transaction, if it contains only one split in
        // the same coin as this split with a positive amount, then the fee is
        // the difference between the amounts
        if ((txn->Splits().size() == 1) &&
            (txn->Splits()[0]->GetCoin()->Id() == coin->Id()) &&
            (txn->Splits()[0]->GetAmount() > 0)) {
          // fee will be a positive amount
          fee = -(amount + txn->Splits()[0]->GetAmount());
        } else {
          printf("WARNING: Couldn't determine fee of transaction %s\n",
              txn->Import_id().c_str());
        }
      } else {
        // get fee from PriceSource
        fee = PriceSource::GetFee(coin, id);
        if (fee == 0) {
          printf("WARNING: Couldn't determine fee of transaction %s\n",
              tx_id.c_str());
        }
      }

      if (fee > 0) {
        splits.push_back(ProtoSplit(fee_account, "", fee, coin, split_id));
      } else if (fee < 0) {
        printf(
            "WARNING: Got a negative fee for transaction %s\n", tx_id.c_str());
      }
    }

    if (mined) {
      // add matching split in mining income
      splits.push_back(ProtoSplit(mining_account, "", -amount, coin, split_id));
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