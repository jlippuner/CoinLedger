/// \file File.cpp
/// \author jlippuner
/// \since Mar 21, 2018
///
/// \brief
///
///

#include "File.hpp"

#include <stdexcept>

#include <sqlite3.h>
#include <boost/filesystem.hpp>

#include "Datetime.hpp"
#include "prices/PriceSource.hpp"

// convenience macros for sqlite3 calls
#define SQL3(db, COMMAND)                                         \
  if ((COMMAND) != SQLITE_OK) {                                   \
    printf("ERROR: SQL error at %s:%i: %s\n", __FILE__, __LINE__, \
        sqlite3_errmsg(db));                                      \
    { SQL3_FAIL; }                                                \
  }

#define SQL3_EXEC(db, sql, callback, callback_arg)                           \
  {                                                                          \
    char* error_msg = nullptr;                                               \
    if (sqlite3_exec(db, sql, callback, callback_arg, &error_msg) !=         \
        SQLITE_OK) {                                                         \
      printf(                                                                \
          "ERROR: SQL error at %s:%i: %s\n", __FILE__, __LINE__, error_msg); \
      sqlite3_free(error_msg);                                               \
      { SQL3_FAIL; }                                                         \
    }                                                                        \
  }

inline int sqlite3_bind_str(
    sqlite3_stmt* stmt, int pos, const std::string& str) {
  return sqlite3_bind_text(stmt, pos, str.c_str(), -1, SQLITE_TRANSIENT);
}

inline std::string sqlite3_column_str(sqlite3_stmt* stmt, int iCol) {
  const unsigned char* ptr = sqlite3_column_text(stmt, iCol);
  if (ptr == nullptr)
    return "";
  else
    return std::string((const char*)ptr);
}

File File::InitNewFile() {
  File file;

  // create root accounts
  Account::Create(&file, "Assets", true, nullptr, false);
  Account::Create(&file, "Liabilities", true, nullptr, false);
  Account::Create(&file, "Income", true, nullptr, false);
  Account::Create(&file, "Expenses", true, nullptr, false);
  Account::Create(&file, "Equity", true, nullptr, false);

  // add all known coins
  Coin::Create(&file, Coin::USD_id(), "US Dollar", "USD");
  PriceSource::AddAllCoins(&file);

  return file;
}

#define SQL3_FAIL throw std::runtime_error("Could not open file " + path)
File File::Open(const std::string& path) {
  File file;

  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) !=
      SQLITE_OK) {
    printf("ERROR: Could not open file '%s' for reading: %s\n", path.c_str(),
        sqlite3_errmsg(db));
    { SQL3_FAIL; }
  }

  // read coins
  {
    sqlite3_stmt* stmt = nullptr;
    SQL3(
        db, sqlite3_prepare_v2(db, "SELECT * FROM coins;", -1, &stmt, nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      std::string id = sqlite3_column_str(stmt, 0);
      std::string name = sqlite3_column_str(stmt, 1);
      std::string symbol = sqlite3_column_str(stmt, 2);

      auto c = file.coins_
                   .emplace(id, std::make_shared<Coin>(Coin(id, name, symbol)))
                   .first->second;
      file.coin_by_symbol_.insert({{c->Symbol(), c}});

      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read accounts
  {
    sqlite3_stmt* stmt = nullptr;
    SQL3(db,
        sqlite3_prepare_v2(db, "SELECT * FROM accounts;", -1, &stmt, nullptr));

    std::unordered_map<uuid_t, uuid_t, uuid_t::hash> parents;

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      std::string name = sqlite3_column_str(stmt, 1);
      bool placeholder = (bool)sqlite3_column_int(stmt, 2);
      uuid_t parent_id = sqlite3_column_uuid(stmt, 3);
      bool single_coin = (bool)sqlite3_column_int(stmt, 4);
      std::string coin_id = sqlite3_column_str(stmt, 5);

      // save parent so we can later connect the accounts
      parents.insert({{id, parent_id}});

      std::shared_ptr<const Coin> coin = nullptr;
      if (single_coin) {
        if (coin_id == "")
          throw std::runtime_error(
              "Account " + name + " has single_coin but no coin is set");
        coin = file.coins_.at(coin_id);
      }

      file.accounts_.emplace(id, std::make_shared<Account>(Account(id, name,
                                     placeholder, nullptr, single_coin, coin)));

      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));

    // set account parents
    for (auto& entry : file.accounts_) {
      auto& accnt = entry.second;
      uuid_t parent_id = parents.at(accnt->Id());

      if (!parent_id.is_nil()) {
        auto parent = file.accounts_.at(parent_id);
        accnt->SetParent(parent);
        parent->AddChild(accnt);
      }
    }

    // make map of full names
    for (auto& entry : file.accounts_) {
      auto& accnt = entry.second;
      file.accounts_by_fullname_.insert({{accnt->FullName(), accnt}});
    }
  }

  // read transactions
  {
    sqlite3_stmt* stmt = nullptr;
    SQL3(db, sqlite3_prepare_v2(
                 db, "SELECT * FROM transactions;", -1, &stmt, nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      Datetime date = sqlite3_column_datetime(stmt, 1);
      std::string description = sqlite3_column_str(stmt, 2);
      std::string import_id = sqlite3_column_str(stmt, 3);

      auto txn = file.transactions_
                     .emplace(id, std::make_shared<Transaction>(Transaction(
                                      id, date, description, import_id)))
                     .first->second;
      file.transactions_by_import_id_.insert({{import_id, txn}});
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read splits
  {
    sqlite3_stmt* stmt = nullptr;
    SQL3(db,
        sqlite3_prepare_v2(db, "SELECT * FROM splits;", -1, &stmt, nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      uuid_t transaction_id = sqlite3_column_uuid(stmt, 1);
      uuid_t account_id = sqlite3_column_uuid(stmt, 2);
      std::string memo = sqlite3_column_str(stmt, 3);
      Amount amount = sqlite3_column_amount(stmt, 4);
      std::string coin_id = sqlite3_column_str(stmt, 5);
      std::string import_id = sqlite3_column_str(stmt, 6);

      auto transaction = file.transactions_.at(transaction_id);
      auto account = file.accounts_.at(account_id);
      auto coin = file.coins_.at(coin_id);

      auto iter = file.splits_.emplace(id,
          std::make_shared<Split>(
              Split(id, transaction, account, memo, amount, coin, import_id)));
      transaction->AddSplit(iter.first->second);
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read daily data
  {
    std::unordered_map<std::string, int64_t> start_days;

    // read meta data
    {
      sqlite3_stmt* stmt = nullptr;
      SQL3(db, sqlite3_prepare_v2(
                   db, "SELECT * FROM daily_data;", -1, &stmt, nullptr));

      int res = sqlite3_step(stmt);
      while (res == SQLITE_ROW) {
        std::string coin_id = sqlite3_column_str(stmt, 0);
        int64_t start_day = sqlite3_column_int64(stmt, 1);
        start_days.insert({{coin_id, start_day}});
        res = sqlite3_step(stmt);
      }
      if (res != SQLITE_DONE) SQL3(db, res);
      SQL3(db, sqlite3_finalize(stmt));
    }

    // read price data
    for (auto& itm : start_days) {
      auto coin_id = itm.first;
      std::vector<Amount> prices;

      sqlite3_stmt* stmt = nullptr;
      std::string sql = "SELECT * FROM [" + coin_id + "_daily_data];";
      SQL3(db, sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr));

      int res = sqlite3_step(stmt);
      while (res == SQLITE_ROW) {
        Amount price = sqlite3_column_amount(stmt, 0);
        prices.push_back(price);
        res = sqlite3_step(stmt);
      }
      if (res != SQLITE_DONE) SQL3(db, res);
      SQL3(db, sqlite3_finalize(stmt));

      auto coin = file.coins_.at(coin_id);
      file.daily_data_.insert({{coin_id, DailyData(coin, itm.second, prices)}});
    }
  }

  SQL3(db, sqlite3_close_v2(db));
  return file;
}
#undef SQL3_FAIL

#define SQL3_FAIL return
void File::Save(const std::string& path) const {
  // if a file with this name already exists, move it to <name>_date
  if (boost::filesystem::exists(path)) {
    auto backup = path + "_" + Datetime::Now().ToStrLocalFile();
    boost::filesystem::rename(path, backup);
  }

  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.c_str(), &db,
          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
    printf("ERROR: Could not open file '%s' for writing: %s\n", path.c_str(),
        sqlite3_errmsg(db));
    return;
  }

  // create tables
  {
    SQL3_EXEC(db, R"(
        CREATE TABLE coins (
          id          TEXT PRIMARY KEY,
          name        TEXT,
          symbol      TEXT
        ) WITHOUT ROWID;
      )",
        nullptr, nullptr);

    SQL3_EXEC(db, R"(
        CREATE TABLE accounts (
          id          BLOB(16) PRIMARY KEY,
          name        TEXT,
          placeholder BOOLEAN,
          parent_id   BLOB(16),
          single_coin BOOLEAN,
          coin        TEXT
        ) WITHOUT ROWID;
      )",
        nullptr, nullptr);

    SQL3_EXEC(db, R"(
        CREATE TABLE transactions (
          id          BLOB(16) PRIMARY KEY,
          date        BLOB,
          description TEXT,
          import_id   TEXT
        ) WITHOUT ROWID;
      )",
        nullptr, nullptr);

    // transaction is a reserved SQL keyword
    SQL3_EXEC(db, R"(
        CREATE TABLE splits (
          id              BLOB(16) PRIMARY KEY,
          transaction_id  BLOB(16),
          account_id      BLOB(16),
          memo            TEXT,
          amount          BLOB,
          coin            TEXT,
          import_id       TEXT
        ) WITHOUT ROWID;
      )",
        nullptr, nullptr);
  }

  // write coins
  {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO coins
        VALUES (?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : coins_) {
      auto c = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement
      SQL3(db, sqlite3_bind_str(stmt, 1, c->Id()));
      SQL3(db, sqlite3_bind_str(stmt, 2, c->Name()));
      SQL3(db, sqlite3_bind_str(stmt, 3, c->Symbol()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write accounts
  {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO accounts
        VALUES (?, ?, ?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : accounts_) {
      auto a = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, a->Id()));
      SQL3(db, sqlite3_bind_str(stmt, 2, a->Name()));
      SQL3(db, sqlite3_bind_int(stmt, 3, a->Placeholder()));

      if (a->Parent() == nullptr) {
        SQL3(db, sqlite3_bind_null(stmt, 4));
      } else {
        SQL3(db, sqlite3_bind_uuid(stmt, 4, a->Parent()->Id()));
      }

      SQL3(db, sqlite3_bind_int(stmt, 5, a->SingleCoin()));

      if (a->GetCoin() == nullptr) {
        SQL3(db, sqlite3_bind_null(stmt, 6));
      } else {
        SQL3(db, sqlite3_bind_str(stmt, 6, a->GetCoin()->Id()));
      }

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write transactions
  {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO transactions
        VALUES (?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : transactions_) {
      auto t = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, t->Id()));
      SQL3(db, sqlite3_bind_datetime(stmt, 2, t->Date()));
      SQL3(db, sqlite3_bind_str(stmt, 3, t->Description()));
      SQL3(db, sqlite3_bind_str(stmt, 4, t->Import_id()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write splits
  {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO splits
        VALUES (?, ?, ?, ?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : splits_) {
      auto s = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, s->Id()));
      SQL3(db, sqlite3_bind_uuid(stmt, 2, s->GetTransaction()->Id()));
      SQL3(db, sqlite3_bind_uuid(stmt, 3, s->GetAccount()->Id()));
      SQL3(db, sqlite3_bind_str(stmt, 4, s->Memo()));
      SQL3(db, sqlite3_bind_amount(stmt, 5, s->GetAmount()));
      SQL3(db, sqlite3_bind_str(stmt, 6, s->GetCoin()->Id()));
      SQL3(db, sqlite3_bind_str(stmt, 7, s->Import_id()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write daily data
  {
    SQL3_EXEC(db, R"(
        CREATE TABLE daily_data (
          coin_id     TEXT PRIMARY KEY,
          start_day   INT8
        ) WITHOUT ROWID;
      )",
        nullptr, nullptr);

    for (auto& itm : daily_data_) {
      auto coin_id = itm.first;
      std::string sql =
          "CREATE TABLE [" + coin_id + "_daily_data] (price BLOB);";
      SQL3_EXEC(db, sql.c_str(), nullptr, nullptr);
    }

    // write meta data
    {
      sqlite3_stmt* stmt = nullptr;
      const char* sql = R"(
        INSERT INTO daily_data
        VALUES (?, ?);
      )";

      SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

      // do inserts inside a transaction, otherwise they're VERY slow
      SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

      for (auto& itm : daily_data_) {
        // first reset statement
        SQL3(db, sqlite3_reset(stmt));

        // bind values to statement
        SQL3(db, sqlite3_bind_str(stmt, 1, itm.first));
        SQL3(db, sqlite3_bind_int64(stmt, 2, itm.second.StartDay()));

        // execute the statement
        int res = sqlite3_step(stmt);
        if (res != SQLITE_DONE) SQL3(db, res);
      }

      SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

      SQL3(db, sqlite3_finalize(stmt));
    }

    // write price data
    for (auto& itm : daily_data_) {
      sqlite3_stmt* stmt = nullptr;
      std::string sql = "INSERT INTO [" + itm.first + "_daily_data] VALUES (?);";

      SQL3(db, sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr));

      // do inserts inside a transaction, otherwise they're VERY slow
      SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

      for (auto p : itm.second.Prices()) {
        // first reset statement
        SQL3(db, sqlite3_reset(stmt));

        // bind values to statement
        SQL3(db, sqlite3_bind_amount(stmt, 1, p));

        // execute the statement
        int res = sqlite3_step(stmt);
        if (res != SQLITE_DONE) SQL3(db, res);
      }

      SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

      SQL3(db, sqlite3_finalize(stmt));
    }
  }

  SQL3(db, sqlite3_close_v2(db));
}
#undef SQL3_FAIL

std::shared_ptr<const Coin> File::GetCoinBySymbol(std::string symbol) const {
  auto count = coin_by_symbol_.count(symbol);
  if (count == 0) {
    printf("WARNING: No coin with symbol '%s' exists\n", symbol.c_str());
    return nullptr;
  } else if (count == 1) {
    return coin_by_symbol_.find(symbol)->second;
  } else {
    printf(
        "WARNING: %lu coins with symbol '%s' exist:\n", count, symbol.c_str());
    auto range = coin_by_symbol_.equal_range(symbol);
    std::for_each(range.first, range.second,
        [=](const decltype(coin_by_symbol_)::value_type& x) {
          auto c = x.second;
          printf("  %s: %s (id = %s)\n", c->Symbol().c_str(), c->Name().c_str(),
              c->Id().c_str());
        });
    auto res = range.first->second;
    printf(
        "  returning %s (id = %s)\n", res->Name().c_str(), res->Id().c_str());
    return res;
  }
}

std::shared_ptr<Transaction> File::GetTransactionFromImportId(
    const std::string& import_id, bool fail_if_not_exist) {
  auto count = transactions_by_import_id_.count(import_id);
  if (count == 0) {
    if (fail_if_not_exist)
      throw std::invalid_argument(
          "No transaction with import id '" + import_id + "' exists");
    else
      return nullptr;
  } else if (count == 1) {
    return transactions_by_import_id_.find(import_id)->second;
  }
  throw std::invalid_argument("There are " + std::to_string(count) +
                              " transactions with import id '" + import_id +
                              "'");
}

void File::BalanceTransaction(
    const std::string& txn_import_id, std::shared_ptr<const Account> account) {
  // first check that the transaction exists, that it is unbalanced, and that it
  // is a single coin transaction

  auto txn = GetTransactionFromImportId(txn_import_id);

  if (txn->Balanced())
    throw std::invalid_argument(
        "Transaction " + txn_import_id + " is already balanced");

  auto coin = txn->GetCoin();
  if (coin == nullptr)
    throw std::invalid_argument(
        "Cannot balance the multi-coin transaction " + txn_import_id);

  Amount total = 0;
  for (auto& sp : txn->Splits()) total += sp->GetAmount();

  auto split = Split::Create(
      this, txn, account, "", -total, coin, "auto_balance_" + txn_import_id);
  txn->AddSplit(split);
}

void File::PrintAccountTree() const {
  GetAccount("Assets")->PrintTree();
  GetAccount("Equity")->PrintTree();
  GetAccount("Expenses")->PrintTree();
  GetAccount("Income")->PrintTree();
  GetAccount("Liabilities")->PrintTree();
}

void File::PrintTransactions() const {
  std::vector<std::shared_ptr<Transaction>> txns;
  for (auto& e : transactions_) txns.push_back(e.second);
  PrintTransactions(txns);
}

void File::PrintUnbalancedTransactions() const {
  std::vector<std::shared_ptr<Transaction>> txns;
  for (auto& e : transactions_) {
    if (!e.second->Balanced()) txns.push_back(e.second);
  }
  PrintTransactions(txns, true);
}

void File::PrintUnmatchedTransactions() const {
  std::vector<std::shared_ptr<Transaction>> txns;
  for (auto& e : transactions_) {
    if (!e.second->Matched()) txns.push_back(e.second);
  }
  PrintTransactions(txns, true);
}

UUIDMap<Balance> File::MakeAccountBalances() const {
  UUIDMap<Balance> balances;

  for (auto& a : accounts_) balances.insert({{a.first, Balance()}});

  for (auto& s : splits_)
    balances[s.second->GetAccount()->Id()].AddSplit(s.second);

  return balances;
}

void File::PrintAccountBalances() const {
  auto balances = MakeAccountBalances();
  auto prices = PriceSource::GetUSDPrices();

  GetAccount("Assets")->PrintTreeBalance(balances, "", false, &prices);
  GetAccount("Equity")->PrintTreeBalance(balances, "", true, &prices);
  GetAccount("Expenses")->PrintTreeBalance(balances, "", false, &prices);
  GetAccount("Income")->PrintTreeBalance(balances, "", true, &prices);
  GetAccount("Liabilities")->PrintTreeBalance(balances, "", true, &prices);
}

Amount File::GetHistoricUSDPrice(
    Datetime time, std::shared_ptr<const Coin> coin) const {
  if (coin->IsUSD()) return 1;
  if (daily_data_.count(coin->Id()) == 0)
    daily_data_.insert({{coin->Id(), DailyData(coin)}});
  return daily_data_.at(coin->Id())(time);
}

void File::PrintTransactions(std::vector<std::shared_ptr<Transaction>> txns,
    bool print_import_id) const {
  // sort transaction by account full name
  std::sort(
      txns.begin(), txns.end(), [](const std::shared_ptr<Transaction>& a,
                                    const std::shared_ptr<Transaction>& b) {
        return a->Date() < b->Date();
      });

  for (auto& txn : txns) txn->Print(print_import_id);
}
