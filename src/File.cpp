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


  SQL3_EXEC(db, R"(
      CREATE TABLE transactions (
        id          BLOB(16) PRIMARY KEY,
        date        UNSIGNED BIGINT,
        description TEXT,
        import_id   TEXT
      ) WITHOUT ROWID;
    )", nullptr, nullptr);


  // write data
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
      SQL3(db, sqlite3_bind_text(stmt, 1, c.Id().c_str(), -1,
          SQLITE_TRANSIENT));

      SQL3(db, sqlite3_bind_text(stmt, 2, c.Name().c_str(), -1,
          SQLITE_TRANSIENT));

      SQL3(db, sqlite3_bind_text(stmt, 3, c.Symbol().c_str(), -1,
          SQLITE_TRANSIENT));

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

  sqlite3_close(db);
}
