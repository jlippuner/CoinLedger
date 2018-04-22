/// \file MiningPoolHub.cpp
/// \author jlippuner
/// \since Apr 22, 2018
///
/// \brief
///
///

#include "MiningPoolHub.hpp"

#include <unordered_map>

#include "CSV.hpp"

namespace {

// There are two types of transactions:
//
// Mining income, which looks like
//   mining_income_account    -(income + fee)   << from Credit* transactions
//   pool_account             +income           << automatically generated
//   mining_fee_account       +fee              << from Fee transactions
// (the fee part may be missing)
//
// Payment to wallet, which looks like
//   wallet                   +payment          << not generated here
//   pool_account             -(payment + fee)  << from Debit_MP and TXFee
//   transaction_fee_account  +fee              << from TXFee transaction

enum class MPHTransactionType { Empty, MiningIncome, PaymentToWallet };

struct MPHTransaction {
  MPHTransaction()
      : import_id(""),
        type(MPHTransactionType::Empty),
        date(Datetime::Now()),
        description(""),
        amount_split(nullptr),
        fee_split(nullptr) {}
  std::string import_id;
  MPHTransactionType type;
  Datetime date;
  std::string description;
  std::shared_ptr<ProtoSplit> amount_split;
  std::shared_ptr<ProtoSplit> fee_split;
};

}  // unnamed namespace

void MiningPoolHub::Import(const std::string& import_file, File* file,
    std::shared_ptr<const Coin> coin, std::shared_ptr<Account> pool_account,
    std::shared_ptr<Account> mining_income_account,
    std::shared_ptr<Account> mining_fee_account,
    std::shared_ptr<Account> transaction_fee_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"ID", "Date", "TX Type", "Status",
      "Payment Address", "TX #", "Block #", "Amount"};
  if (csv.Header() != expected_header)
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  std::unordered_map<std::string, MPHTransaction> transactions;

  for (auto& rec : csv.Content()) {
    auto txn_id = rec[0];
    auto time = Datetime::FromMiningPoolHubUTC(rec[1]);
    auto type = rec[2];
    auto status = rec[3];
    auto addr = rec[4];
    auto txn_hash = rec[5];
    auto block = rec[6];
    auto amount = Amount::Parse(rec[7]);

    if ((status == "Unconfirmed") || (status == "Orphaned"))
      continue;
    else if (status != "Confirmed")
      throw std::runtime_error("Unknown status " + status);

    std::string split_id = "MiningPoolHub_" + coin->Id() + "_" + txn_id;

    if ((type == "Credit") || (type == "Credit_AE") || (type == "Fee")) {
      if ((block == "n/a") || (block == "")) {
        if (type == "Fee")
          throw std::runtime_error("Got empty block with a Fee transaction");
        else
          continue;
      }

      std::string tx_id = "MiningPoolHub_" + coin->Id() + "_" + block;
      transactions[tx_id].import_id = tx_id;
      transactions[tx_id].type = MPHTransactionType::MiningIncome;
      transactions[tx_id].date = time;
      transactions[tx_id].description =
          "MiningPoolHub " + coin->Id() + " mining";

      if (type == "Fee") {
        if (amount >= 0)
          throw std::runtime_error("Expect fee amount to be negative");
        transactions[tx_id].fee_split = std::shared_ptr<ProtoSplit>(
            new ProtoSplit(mining_fee_account, "", -amount, coin, split_id));
      } else {
        if (amount <= 0)
          throw std::runtime_error("Expect credit amount to be positive");
        transactions[tx_id].amount_split = std::shared_ptr<ProtoSplit>(
            new ProtoSplit(mining_income_account, "", -amount, coin, split_id));
      }
    } else if ((type == "Debit_MP") || (type == "TXFee")) {
      if (addr == "")
        throw std::runtime_error(
            "Need a payment address for Debit_MP and TXFee");

      if (amount >= 0)
        throw std::runtime_error(
            "Expect a negative amount for Debit_MP and TXFee");

      std::string tx_id = "MiningPoolHub_" + coin->Id() + "_" + rec[1] + addr;
      transactions[tx_id].type = MPHTransactionType::PaymentToWallet;
      transactions[tx_id].date = time;
      transactions[tx_id].description =
          "MiningPoolHub " + coin->Id() + " payment to wallet " + addr;

      if (type == "Debit_MP") {
        if (txn_hash == "")
          throw std::runtime_error("Need a transaction hash for Debit_MP");
        transactions[tx_id].import_id = coin->Symbol() + "_" + txn_hash;
        transactions[tx_id].amount_split = std::shared_ptr<ProtoSplit>(
            new ProtoSplit(pool_account, "", amount, coin, split_id));
      } else {
        transactions[tx_id].fee_split =
            std::shared_ptr<ProtoSplit>(new ProtoSplit(
                transaction_fee_account, "", -amount, coin, split_id));
      }
    } else if (type == "Debit_AE") {
      if (addr == "Transferred to local account")
        continue;
      else
        std::runtime_error("Don't know how to handle Debit_AE");
    } else if (type == "Debit_AP") {
      std::runtime_error("Don't know how to handle Debit_AP");
    } else {
      throw std::runtime_error("Unknown MiningPoolHub type '" + type + "'");
    }
  }

  for (auto& itm : transactions) {
    auto mph_id = itm.first;
    auto mph_txn = itm.second;
    auto tx_id = mph_txn.import_id;

    // check if a transaction with this import_id already exists
    auto txn = file->GetTransactionFromImportId(tx_id);

    // we will create one to three splits, depending on the type of record
    std::vector<ProtoSplit> splits;

    if (mph_txn.amount_split == nullptr)
      throw std::runtime_error("Cannot have empty amount split");

    if (mph_txn.type == MPHTransactionType::MiningIncome) {
      Amount total = mph_txn.amount_split->amount_;
      splits.push_back(*mph_txn.amount_split);

      if (mph_txn.fee_split != nullptr) {
        total += mph_txn.fee_split->amount_;
        splits.push_back(*mph_txn.fee_split);
      }

      // create balancing split in the pool account
      splits.push_back(
          ProtoSplit(pool_account, "", -total, coin, mph_id + "_balance"));
    } else if (mph_txn.type == MPHTransactionType::PaymentToWallet) {
      Amount fee = 0;
      if (mph_txn.fee_split != nullptr) {
        fee = mph_txn.fee_split->amount_;
        splits.push_back(*mph_txn.fee_split);
      }

      mph_txn.amount_split->amount_ -= fee;
      splits.push_back(*mph_txn.amount_split);
    } else {
      throw std::runtime_error("Unknown transaction type");
    }

    // if a transaction with this import id already exists, check if it has a
    // split with this split id, in which case we don't need to duplicate this
    // split
    std::vector<ProtoSplit> new_splits;
    for (auto& sp : splits) {
      if ((txn != nullptr) && (txn->HasSplitWithImportId(sp.import_id_))) {
        ++num_duplicate;
      } else {
        new_splits.push_back(sp);
        ++num_imported;
      }
    }

    if ((txn == nullptr) && (new_splits.size() > 0)) {
      // we need to create a new transaction
      Transaction::Create(
          file, mph_txn.date, mph_txn.description, new_splits, tx_id);
    } else {
      // the transaction already exists, just add the new splits
      for (auto& proto_s : new_splits) {
        auto split = Split::Create(file, txn, proto_s);
        txn->AddSplit(split);
      }
    }
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}