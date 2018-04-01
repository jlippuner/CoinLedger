/// \file File.cpp
/// \author jlippuner
/// \since Mar 21, 2018
///
/// \brief
///
///

#include "File.hpp"

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

#define SQL3_FAIL return
void File::Save(const std::string path) const {
  sqlite3 * db = nullptr;
  if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
    printf("ERROR: Could not open file '%s' for writing: %s\n", path.c_str(),
        sqlite3_errmsg(db));
    return;
  }

  // Create tables
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
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: SQL error when inserting coin: %s\n",
            sqlite3_errmsg(db));
        return;
      }
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    sqlite3_finalize(stmt);
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
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: SQL error when inserting account: %s\n",
            sqlite3_errmsg(db));
        return;
      }
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    sqlite3_finalize(stmt);
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
      SQL3(db, sqlite3_bind_blob(stmt, 2, t.Date().Raw(), t.Date().size(),
          SQLITE_TRANSIENT));
      SQL3(db, sqlite3_bind_str(stmt, 3, t.Description()));
      SQL3(db, sqlite3_bind_str(stmt, 4, t.Import_id()));

      // execute the statement
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: SQL error when inserting transaction: %s\n",
            sqlite3_errmsg(db));
        return;
      }
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    sqlite3_finalize(stmt);
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
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: SQL error when inserting split: %s\n",
            sqlite3_errmsg(db));
        return;
      }
    }

    SQL3_EXEC(db, "END TRANSACTION;", nullptr, nullptr);

    sqlite3_finalize(stmt);
  }

  sqlite3_close(db);
}
