/// \file ETHECR20Account.cpp
/// \author jlippuner
/// \since Feb 16, 2020
///
/// \brief
///
///

#include "ETHECR20Account.hpp"

#include <json/reader.h>
#include <thread>

#include "Split.hpp"
#include "prices/PriceSource.hpp"

void ETHECR20Account::Import(const std::string& eth_account, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
    std::shared_ptr<Account> eth_account_for_fee,
    const std::map<std::string, std::string>& transaction_associations) {
  if (account->SingleCoin()) {
    throw std::invalid_argument(
        "An Ethereum ECR20 account can only be imported into a multi-coin account");
  }
  // retrieve the transactions for this account
  std::string url =
      "http://api.etherscan.io/api?module=account&action=tokentx&address=" +
      eth_account;

  Json::Reader reader;
  Json::Value root;
  while (true) {
    try {
      auto json = PriceSource::GetURL(url);

      if (!reader.parse(json, root))
        throw std::runtime_error("Could not parse result from " + url);

      if ((root["status"].asString() != "1") &&
          (root["message"].asString() != "OK"))
        throw std::runtime_error("API request failed, url = " + url);

      break;
    } catch (std::exception &e) {
      printf("Accessing etherscan failed, retrying in 5 seconds...\n");
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }

  auto num = root["result"].size();
  if (num >= 10000)
    throw std::runtime_error(
        "ETH ECR20 account may have more than 10,000 transactions, this is not "
        "implemented yet");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  printf("Got %i ECR20 transactions\n", num);

  for (unsigned int i = 0; i < num; ++i) {
    auto json_txn = root["result"][i];

    auto time = Datetime::FromUNIXTimestamp(
        std::stoll(json_txn["timeStamp"].asString()));
    auto id = json_txn["hash"].asString();
    auto src = json_txn["from"].asString();
    auto dest = json_txn["to"].asString();
    int128_t amount_int(json_txn["value"].asString());
    auto symbol = json_txn["tokenSymbol"].asString();
    int decimal = std::stoi(json_txn["tokenDecimal"].asString());
    int128_t gas_price_int(json_txn["gasPrice"].asString());
    int128_t gas_used_int(json_txn["gasUsed"].asString());
    auto confirmations = std::stoll(json_txn["confirmations"].asString());

    if (confirmations < 6) continue;

    Amount amount(amount_int, -decimal);
    Amount fee(gas_price_int * gas_used_int, -18);

    auto coin = file->GetCoinBySymbol(symbol);
    if (!coin) {
      printf("Skipping token transaction for token %s\n", symbol.c_str());
      continue;
    }

    auto eth_coin = file->GetCoinBySymbol("ETH");

    std::string desc = "";
    if (src == eth_account) {
      amount = -amount;
      desc = "Withdrawal";
    } else if (dest == eth_account) {
      desc = "Deposit";
    } else {
      throw std::runtime_error(
          "ETH transaction does not involve the ETH ECR20 account");
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
        splits.push_back(ProtoSplit(fee_account, "", fee, eth_coin, split_id));
        splits.push_back(ProtoSplit(account, "", amount, coin, split_id));
        splits.push_back(ProtoSplit(eth_account_for_fee, "", -fee, eth_coin, split_id));
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
      "Imported %lu records and skipped %lu duplicates from ETH ECR20 account %s\n",
      num_imported, num_duplicate, eth_account.c_str());
}