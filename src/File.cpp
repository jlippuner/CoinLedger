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
#define SQL3_EXEC(db, sql, callback, callback_arg)                              \
  {                                                                             \
    char * error_msg = nullptr;                                                 \
    if (sqlite3_exec(db, sql, callback, callback_arg, &error_msg)               \
        != SQLITE_OK) {                                                         \
      printf("ERROR: SQL error at %s:%s: %s\n", __FILE__, __LINE__, error_msg); \
      sqlite3_free(error_msg);                                                  \
      SQL3_FAIL;                                                                \
    }                                                                           \
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

void File::Save(const std::string path) const {
  sqlite3 * db = nullptr;
  if (sqlite3_open(path.c_str(), &db) != 0) {
    printf("ERROR: Could not open file '%s' for writing: %s\n", path.c_str(),
        sqlite3_errmsg(db));
    return;
  }

  // Create tables
  char * err = nullptr;

  {
    const char * sql = R"(
        CREATE TABLE coins (
          id          TEXT PRIMARY KEY,
          name        TEXT,
          symbol      TEXT
        ) WITHOUT ROWID;
      )";

    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
      printf("ERROR: SQL error when creating coins table: %s\n", err);
      sqlite3_free(err);
      return;
    }
  }

  {
    const char * sql = R"(
        CREATE TABLE accounts (
          id          BLOB(16) PRIMARY KEY,
          name        TEXT,
          placeholder BOOLEAN,
          parent_id   BLOB(16),
          single_coin BOOLEAN,
          coin        TEXT
        ) WITHOUT ROWID;
      )";

    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
      printf("ERROR: SQL error when creating accounts table: %s\n", err);
      sqlite3_free(err);
      return;
    }
  }

  {
    // transaction is a reserved SQL keyword
    const char * sql = R"(
        CREATE TABLE splits (
          id              BLOB(16) PRIMARY KEY,
          transaction_id  BLOB(16),
          account_id      BLOB(16),
          memo            TEXT,
          amount          BIGINT,
          coin            TEXT
        ) WITHOUT ROWID;
      )";

    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
      printf("ERROR: SQL error when creating splits table: %s\n", err);
      sqlite3_free(err);
      return;
    }
  }

  {
    const char * sql = R"(
        CREATE TABLE transactions (
          id          BLOB(16) PRIMARY KEY,
          date        UNSIGNED BIGINT,
          description TEXT,
          import_id   TEXT
        ) WITHOUT ROWID;
      )";

    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
      printf("ERROR: SQL error when creating transactions table: %s\n", err);
      sqlite3_free(err);
      return;
    }
  }

  // write data
  {
    sqlite3_stmt * stmt = nullptr;
    const char * sql = R"(
        INSERT INTO coins
        VALUES (?, ?, ?);
      )";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      printf("ERROR: SQL error when inserting coin: %s\n", sqlite3_errmsg(db));
      return;
    }

    // do inserts inside a transaction, otherwise they're VERY slow
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err)
        != SQLITE_OK) {
      printf("ERROR: SQL error when starting coin insert transaction: %s\n",
          err);
      sqlite3_free(err);
      return;
    }

    for (const auto& itm : coins_) {
      const Coin& c = itm.second;

      // first reset statement
      if (sqlite3_reset(stmt) != SQLITE_OK) {
        printf("ERROR: SQL error when resetting coin insert statement: %s\n",
            sqlite3_errmsg(db));
        return;
      }

      // bind values to statement
      if (sqlite3_bind_text(stmt, 1, c.Id().c_str(), -1, SQLITE_TRANSIENT)
          != SQLITE_OK) {
        printf("ERROR: SQL error when binding coin id: %s\n",
            sqlite3_errmsg(db));
        return;
      }

      if (sqlite3_bind_text(stmt, 2, c.Name().c_str(), -1, SQLITE_TRANSIENT)
          != SQLITE_OK) {
        printf("ERROR: SQL error when binding coin name: %s\n",
            sqlite3_errmsg(db));
        return;
      }

      if (sqlite3_bind_text(stmt, 3, c.Symbol().c_str(), -1, SQLITE_TRANSIENT)
          != SQLITE_OK) {
        printf("ERROR: SQL error when binding coin symbol: %s\n",
            sqlite3_errmsg(db));
        return;
      }

      // execute the statement
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("ERROR: SQL error when inserting coin: %s\n",
            sqlite3_errmsg(db));
        return;
      }
    }

    if (sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, &err)
        != SQLITE_OK) {
      printf("ERROR: SQL error when ending coin insert transaction: %s\n",
          err);
      sqlite3_free(err);
      return;
    }

    sqlite3_finalize(stmt);
  }

  sqlite3_close(db);
}
