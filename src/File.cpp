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

#include "PriceSource.hpp"

// convenience macros for sqlite3 calls
#define SQL3(db, COMMAND)                                                       \
  if ((COMMAND) != SQLITE_OK) {                                                 \
    printf("ERROR: SQL error at %s:%i: %s\n", __FILE__, __LINE__,               \
        sqlite3_errmsg(db));                                                    \
    {SQL3_FAIL;}                                                                \
  }                                                                             \


#define SQL3_EXEC(db, sql, callback, callback_arg)                              \
  {                                                                             \
    char * error_msg = nullptr;                                                 \
    if (sqlite3_exec(db, sql, callback, callback_arg, &error_msg)               \
        != SQLITE_OK) {                                                         \
      printf("ERROR: SQL error at %s:%i: %s\n", __FILE__, __LINE__, error_msg); \
      sqlite3_free(error_msg);                                                  \
      {SQL3_FAIL;}                                                              \
    }                                                                           \
  }

inline int sqlite3_bind_str(sqlite3_stmt * stmt, int pos,
    const std::string& str) {
  return sqlite3_bind_text(stmt, pos, str.c_str(), -1, SQLITE_TRANSIENT);
}

inline std::string sqlite3_column_str(sqlite3_stmt * stmt, int iCol) {
  const unsigned char * ptr = sqlite3_column_text(stmt, iCol);
  if (ptr == nullptr)
    return "";
  else
    return std::string((const char*)ptr);
}

File File::InitNewFile() {
  File file;

  // create root accounts
  Account::Create(&file, "Assets",      true, nullptr, false);
  Account::Create(&file, "Liabilities", true, nullptr, false);
  Account::Create(&file, "Income",      true, nullptr, false);
  Account::Create(&file, "Expenses",    true, nullptr, false);
  Account::Create(&file, "Equity",      true, nullptr, false);

  // add all known coins
  PriceSource::AddAllCoins(&file);

  return file;
}

#define SQL3_FAIL throw std::runtime_error("Could not open file " + path)
File File::Open(const std::string& path) {
  File file;

  sqlite3 * db = nullptr;
  if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr)
      != SQLITE_OK) {
    printf("ERROR: Could not open file '%s' for reading: %s\n", path.c_str(),
        sqlite3_errmsg(db));
    {SQL3_FAIL;}
  }

  // read coins
  {
    sqlite3_stmt * stmt = nullptr;
    SQL3(db, sqlite3_prepare_v2(db, "SELECT * FROM coins;", -1, &stmt,
        nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      std::string id = sqlite3_column_str(stmt, 0);
      std::string name = sqlite3_column_str(stmt, 1);
      std::string symbol = sqlite3_column_str(stmt, 2);

      file.coins_.emplace(id, Coin(id, name, symbol));
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read accounts
  {
    sqlite3_stmt * stmt = nullptr;
    SQL3(db, sqlite3_prepare_v2(db, "SELECT * FROM accounts;", -1, &stmt,
        nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      std::string name = sqlite3_column_str(stmt, 1);
      bool placeholder = (bool)sqlite3_column_int(stmt, 2);
      uuid_t parent_id = sqlite3_column_uuid(stmt, 3);
      bool single_coin = (bool)sqlite3_column_int(stmt, 4);
      std::string coin_id = sqlite3_column_str(stmt, 5);

      const Account * parent = nullptr;
      const Coin * coin = nullptr;

      if (!parent_id.is_nil())
        parent = &file.accounts_.at(parent_id);

      if (single_coin) {
        if (coin_id == "")
          throw std::runtime_error("Account " + name + " has single_coin but "
              "no coin is set");
        coin = &file.coins_.at(coin_id);
      }

      file.accounts_.emplace(id, Account(id, name, placeholder, parent,
          single_coin, coin));
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read transactions
  {
    sqlite3_stmt * stmt = nullptr;
    SQL3(db, sqlite3_prepare_v2(db, "SELECT * FROM transactions;", -1, &stmt,
        nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      Datetime date = sqlite3_column_datetime(stmt, 1);
      std::string description = sqlite3_column_str(stmt, 2);
      std::string import_id = sqlite3_column_str(stmt, 3);

      file.transactions_.emplace(id, Transaction(id, date, description,
          import_id));
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  // read splits
  {
    sqlite3_stmt * stmt = nullptr;
    SQL3(db, sqlite3_prepare_v2(db, "SELECT * FROM splits;", -1, &stmt,
        nullptr));

    int res = sqlite3_step(stmt);
    while (res == SQLITE_ROW) {
      uuid_t id = sqlite3_column_uuid(stmt, 0);
      uuid_t transaction_id = sqlite3_column_uuid(stmt, 1);
      uuid_t account_id = sqlite3_column_uuid(stmt, 2);
      std::string memo = sqlite3_column_str(stmt, 3);
      Amount amount = Amount::FromRaw(sqlite3_column_int64(stmt, 4));
      std::string coin_id = sqlite3_column_str(stmt, 5);

      auto transaction = &file.transactions_.at(transaction_id);
      auto account = &file.accounts_.at(account_id);
      auto coin = &file.coins_.at(coin_id);

      auto iter = file.splits_.emplace(id, Split(id, transaction, account,
          memo, amount, coin));
      transaction->AddSplit(&iter.first->second);
      res = sqlite3_step(stmt);
    }
    if (res != SQLITE_DONE) SQL3(db, res);
    SQL3(db, sqlite3_finalize(stmt));
  }

  SQL3(db, sqlite3_close_v2(db));
  return file;
}
#undef SQL3_FAIL

#define SQL3_FAIL return
void File::Save(const std::string& path) const {
  sqlite3 * db = nullptr;
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
      )", nullptr, nullptr);

    SQL3_EXEC(db, R"(
        CREATE TABLE accounts (
          id          BLOB(16) PRIMARY KEY,
          name        TEXT,
          placeholder BOOLEAN,
          parent_id   BLOB(16),
          single_coin BOOLEAN,
          coin        TEXT
        ) WITHOUT ROWID;
      )", nullptr, nullptr);

    SQL3_EXEC(db, R"(
        CREATE TABLE transactions (
          id          BLOB(16) PRIMARY KEY,
          date        BLOB,
          description TEXT,
          import_id   TEXT
        ) WITHOUT ROWID;
      )", nullptr, nullptr);

    // transaction is a reserved SQL keyword
    SQL3_EXEC(db, R"(
        CREATE TABLE splits (
          id              BLOB(16) PRIMARY KEY,
          transaction_id  BLOB(16),
          account_id      BLOB(16),
          memo            TEXT,
          amount          BIGINT,
          coin            TEXT
        ) WITHOUT ROWID;
      )", nullptr, nullptr);
  }

  // write coins
  {
    sqlite3_stmt * stmt = nullptr;
    const char * sql = R"(
        INSERT INTO coins
        VALUES (?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : coins_) {
      const Coin& c = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement
      SQL3(db, sqlite3_bind_str(stmt, 1, c.Id()));
      SQL3(db, sqlite3_bind_str(stmt, 2, c.Name()));
      SQL3(db, sqlite3_bind_str(stmt, 3, c.Symbol()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write accounts
  {
    sqlite3_stmt * stmt = nullptr;
    const char * sql = R"(
        INSERT INTO accounts
        VALUES (?, ?, ?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : accounts_) {
      const Account& a = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, a.Id()));
      SQL3(db, sqlite3_bind_str(stmt, 2, a.Name()));
      SQL3(db, sqlite3_bind_int(stmt, 3, a.Placeholder()));

      if (a.Parent() == nullptr) {
        SQL3(db, sqlite3_bind_null(stmt, 4));
      } else {
        SQL3(db, sqlite3_bind_uuid(stmt, 4, a.Parent()->Id()));
      }

      SQL3(db, sqlite3_bind_int(stmt, 5, a.Single_coin()));

      if (a.GetCoin() == nullptr) {
        SQL3(db, sqlite3_bind_null(stmt, 6));
      } else {
        SQL3(db, sqlite3_bind_str(stmt, 6, a.GetCoin()->Id()));
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
    sqlite3_stmt * stmt = nullptr;
    const char * sql = R"(
        INSERT INTO transactions
        VALUES (?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : transactions_) {
      const Transaction& t = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, t.Id()));
      SQL3(db, sqlite3_bind_datetime(stmt, 2, t.Date()));
      SQL3(db, sqlite3_bind_str(stmt, 3, t.Description()));
      SQL3(db, sqlite3_bind_str(stmt, 4, t.Import_id()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  // write splits
  {
    sqlite3_stmt * stmt = nullptr;
    const char * sql = R"(
        INSERT INTO splits
        VALUES (?, ?, ?, ?, ?, ?);
      )";

    SQL3(db, sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));

    // do inserts inside a transaction, otherwise they're VERY slow
    SQL3_EXEC(db, "BEGIN TRANSACTION;", nullptr, nullptr);

    for (const auto& itm : splits_) {
      const Split& s = itm.second;

      // first reset statement
      SQL3(db, sqlite3_reset(stmt));

      // bind values to statement

      SQL3(db, sqlite3_bind_uuid(stmt, 1, s.Id()));
      SQL3(db, sqlite3_bind_uuid(stmt, 2, s.GetTransaction()->Id()));
      SQL3(db, sqlite3_bind_uuid(stmt, 3, s.GetAccount()->Id()));
      SQL3(db, sqlite3_bind_str(stmt, 4, s.Memo()));
      SQL3(db, sqlite3_bind_int64(stmt, 5, s.GetAmount().Raw()));
      SQL3(db, sqlite3_bind_str(stmt, 6, s.GetCoin()->Id()));

      // execute the statement
      int res = sqlite3_step(stmt);
      if (res != SQLITE_DONE) SQL3(db, res);
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    SQL3(db, sqlite3_finalize(stmt));
  }

  SQL3(db, sqlite3_close_v2(db));
}
#undef SQL3_FAIL
