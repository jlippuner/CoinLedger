/// \file XRPAccount.cpp
/// \author jlippuner
/// \since Apr 20, 2018
///
/// \brief
///
///

#include "XRPAccount.hpp"

#include <json/reader.h>

#include "PriceSource.hpp"

void XRPAccount::Import(const std::string& xrp_account, File* file,
    std::shared_ptr<Account> account, std::shared_ptr<Account> fee_account,
    const std::map<std::string, std::string>& transaction_associations) {
  if (!account->SingleCoin()) {
    throw std::invalid_argument(
        "A Ripple account can only be imported into a single-coin account");
  }
  auto coin = account->GetCoin();
  if (coin->Id() != "ripple")
    throw std::invalid_argument(
        "A Ripple account can only be imported into an XRP account");

  // retrieve the transactions for this account
  std::string url = "https://data.ripple.com/v2/accounts/" + xrp_account +
                    "/transactions?limit=1000";
  auto json = PriceSource::GetURL(url);

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(json, root))
    throw std::runtime_error("Could not parse result from " + url);

  if (root["result"].asString() != "success")
    throw std::runtime_error("API request failed, url = " + url);

  int num = root["count"].asInt();
  if (!root["marker"].isNull())
    throw std::runtime_error(
        "XRP account has more than 1000 transactions, this is not implemented "
        "yet");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (int i = 0; i < num; ++i) {
    auto json_txn = root["transactions"][i];

    auto id = json_txn["hash"].asString();
    auto time = Datetime::FromXRP(json_txn["date"].asString());
    auto type = json_txn["tx"]["TransactionType"].asString();
    Amount amount(std::stoi(json_txn["tx"]["Amount"].asString()), -6);
    Amount fee(std::stoi(json_txn["tx"]["Fee"].asString()), -6);
    auto src = json_txn["tx"]["Account"].asString();
    auto dest = json_txn["tx"]["Destination"].asString();

    if (type != "Payment")
      throw std::runtime_error("Unknown XRP transaction type '" + type + "'");

    std::string desc = "";
    if (src == xrp_account) {
      amount = -amount;
      desc = "Withdrawal";
    } else if (dest == xrp_account) {
      desc = "Deposit";
    } else {
      throw std::runtime_error(
          "XRP transaction does not involve the XRP account");
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

    // main split in the XRP account
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
      // set the transaction date to the date of the XRP transaction
      txn->SetDate(time);
    }
    ++num_imported;
  }

  printf(
      "Imported %lu records and skipped %lu duplicates from XRP account %s\n",
      num_imported, num_duplicate, xrp_account.c_str());
}