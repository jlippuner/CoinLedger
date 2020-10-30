/// \file Kraken.cpp
/// \author jlippuner
/// \since Apr 08, 2018
///
/// \brief
///
///

#include "Kraken.hpp"

#include "CSV.hpp"

void Kraken::Import(const std::string& import_file, File* file,
    std::shared_ptr<Account> account,
    std::shared_ptr<Account> trading_fee_account,
    std::shared_ptr<Account> transaction_fee_account,
    std::shared_ptr<Account> usd_investment_account) {
  // read the CSV file
  CSV csv(import_file);

  std::vector<std::string> expected_header{"txid", "refid", "time", "type",
      "aclass", "asset", "amount", "fee", "balance"};

  std::vector<std::string> new_expected_header = expected_header;
  new_expected_header.insert(new_expected_header.begin() + 4, "subtype");

  bool new_style = (csv.Header() == new_expected_header);
  if (!new_style && (csv.Header() != expected_header))
    throw std::invalid_argument(
        "CSV file '" + import_file + "' has an unexpected header");

  size_t num_duplicate = 0;
  size_t num_imported = 0;

  for (auto& in_rec : csv.Content()) {
    auto rec = in_rec;
    if (new_style) rec.erase(rec.begin() + 4);

    std::string split_id = "Kraken_" + rec[0];
    std::string tx_id = "Kraken_" + rec[1];
    auto time = Datetime::FromUTC(rec[2]);
    auto type = rec[3];
    auto aclass = rec[4];
    auto coin_str = rec[5];
    auto amount = Amount::Parse(rec[6]);
    auto fee = Amount::Parse(rec[7]);

    if (aclass != "currency")
      throw std::runtime_error("Unknown asset class '" + aclass + "'");

    std::shared_ptr<const Coin> coin = nullptr;
    // Kraken uses non-standard coin symbols, first try if we know this coin
    if (file->CoinBySymbol().count(coin_str) > 0) {
      // we know the coin
      coin = file->GetCoinBySymbol(coin_str);
    } else {
      // try to translate the Kraken coin
      std::string cid;
      if (coin_str == "XBT")
        cid = "bitcoin";
      else if (coin_str == "XETC")
        cid = "ethereum-classic";
      else if (coin_str == "XETH")
        cid = "ethereum";
      else if (coin_str == "XICN")
        cid = "iconomi";
      else if (coin_str == "XLTC")
        cid = "litecoin";
      else if (coin_str == "XMLN")
        cid = "melon";
      else if (coin_str == "XREP")
        cid = "augur";
      else if (coin_str == "XXBT")
        cid = "bitcoin";
      else if (coin_str == "XXDG")
        cid = "dogecoin";
      else if (coin_str == "XXLM")
        cid = "stellar";
      else if (coin_str == "XXMR")
        cid = "monero";
      else if (coin_str == "XXRP")
        cid = "ripple";
      else if (coin_str == "XZEC")
        cid = "zcash";
      else if (coin_str == "ZUSD")
        cid = Coin::USD_id();
      else
        throw std::invalid_argument("Unknown Kraken coin '" + coin_str + "'");

      coin = file->Coins().at(cid);
    }

    if (coin == nullptr)
      throw std::invalid_argument("Unknown coin '" + coin_str + "'");

    // make sure we understand the fee coin
    if (fee != 0) {
      // if there is a fee, the transaction must either be a deposit/withdrawal,
      // or it must be the USD part of the trade
      if ((type == "trade") && (!coin->IsUSD()))
        throw std::invalid_argument("Kraken trade with unknown fee coin");
    }

    std::string tx_description = "Kraken ";

    if ((type == "deposit") || (type == "withdrawal")) {
      tx_description += coin->Symbol() + " " + type;
    } else if (type == "trade") {
      tx_description += "trade";
    } else {
      throw std::runtime_error("Unknown Kraken type '" + type + "'");
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

    // main split in the Kraken account
    splits.push_back(ProtoSplit(account, "", amount, coin, split_id));

    if (fee != 0) {
      if (type == "trade") {
        // Kraken fees are positive
        splits.push_back(ProtoSplit(account, "", -fee, coin, split_id));
        splits.push_back(
            ProtoSplit(trading_fee_account, "", fee, coin, split_id));
      } else {
        // it's a transaction fee
        splits.push_back(ProtoSplit(account, "", -fee, coin, split_id));
        splits.push_back(
            ProtoSplit(transaction_fee_account, "", fee, coin, split_id));
      }
    }

    if (coin->IsUSD() && ((type == "deposit") || (type == "withdrawal"))) {
      // if this is a USD deposit or withdrawal, we can add the matching split
      // in the USD investment account
      splits.push_back(
          ProtoSplit(usd_investment_account, "", -amount, coin, split_id));
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
  }

  printf("Imported %lu records and skipped %lu duplicates from %s\n",
      num_imported, num_duplicate, import_file.c_str());
}