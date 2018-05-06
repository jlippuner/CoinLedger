/// \file ETHAccount.cpp
/// \author jlippuner
/// \since Apr 21, 2018
///
/// \brief
///
///

#include "ETHAccount.hpp"

#include <json/reader.h>

#include "prices/PriceSource.hpp"

void ETHAccount::Import(const std::string& eth_account, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
    const std::map<std::string, std::string>& transaction_associations) {
  if (!account->SingleCoin()) {
    throw std::invalid_argument(
        "An Ethereum account can only be imported into a single-coin account");
  }
  auto coin = account->GetCoin();
  if (coin->Id() != "ethereum")
    throw std::invalid_argument(
        "An Ethereum account can only be imported into an ETH account");

  // retrieve the transactions for this account
  std::string url =
      "http://api.etherscan.io/api?module=account&action=txlist&address=" +
      eth_account;
  auto json = PriceSource::GetURL(url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + url);

  if ((root["status"].asString() != "1") &&
      (root["message"].asString() != "OK"))
    throw std::runtime_error("API request failed, url = " + url);

  auto num = root["result"].size();
  if (num >= 10000)
    throw std::runtime_error(
        "ETH account may have more than 10,000 transactions, this is not "
        "implemented yet");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (unsigned int i = 0; i < num; ++i) {
    auto json_txn = root["result"][i];

    auto time = Datetime::FromUNIXTimestamp(
        std::stoll(json_txn["timeStamp"].asString()));
    auto id = json_txn["hash"].asString();
    auto src = json_txn["from"].asString();
    auto dest = json_txn["to"].asString();
    int128_t amount_int(json_txn["value"].asString());
    int128_t gas_price_int(json_txn["gasPrice"].asString());
    auto err = json_txn["isError"].asString();
    int128_t gas_used_int(json_txn["gasUsed"].asString());
    auto confirmations = std::stoll(json_txn["confirmations"].asString());

    if ((err != "0") || (confirmations < 6)) continue;

    Amount amount(amount_int, -18);
    Amount fee(gas_price_int * gas_used_int, -18);

    std::string desc = "";
    if (src == eth_account) {
      amount = -amount;
      desc = "Withdrawal";
    } else if (dest == eth_account) {
      desc = "Deposit";
    } else {
      throw std::runtime_error(
          "ETH transaction does not involve the ETH account");
    }

    std::string tx_id = coin->Symbol() + "_" + id;
    if (transaction_associations.count(id) > 0) {
      tx_id = transaction_associations.at(id);
    }

    std::string split_id = coin->Symbol() + "_" + desc + "_" + id;
    std::string tx_description = coin->Symbol() + " " + desc;

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

    // if this split sends out money, account for the fee
    if (amount < 0) {
      if (fee > 0) {
        splits.push_back(ProtoSplit(fee_account, "", fee, coin, split_id));
        splits.push_back(ProtoSplit(account, "", amount - fee, coin, split_id));
      } else if (fee == 0) {
        splits.push_back(ProtoSplit(account, "", amount, coin, split_id));
      } else if (fee < 0) {
        printf(
            "WARNING: Got a negative fee for transaction %s\n", tx_id.c_str());
      }
    } else if (amount > 0) {
      splits.push_back(ProtoSplit(account, "", amount, coin, split_id));
    }

    if (txn == nullptr) {
      // json_txn need to create a new transaction
      Transaction::Create(file, time, tx_description, splits, tx_id);
    } else {
      // the transaction already exists, just add the new splits
      for (auto& proto_s : splits) {
        auto split = Split::Create(file, txn, proto_s);
        txn->AddSplit(split);
      }
      // set the transaction date to the date of the ETH transaction
      txn->SetDate(time);
    }
    ++num_imported;
  }

  printf(
      "Imported %lu records and skipped %lu duplicates from ETH account %s\n",
      num_imported, num_duplicate, eth_account.c_str());
}