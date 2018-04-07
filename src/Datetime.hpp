/// \file Datetime.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_DATETIME_HPP_
#define SRC_DATETIME_HPP_

#include <ctime>
#include <stdexcept>

#include <sqlite3.h>

// Represents a point in time. The resolution is seconds and internally the date
// and time are stored as a unix timestamp in UTC

class Datetime {
public:
  static Datetime FromRaw(const void* ptr) {
    return Datetime(*((time_t*)ptr));
  }

  static Datetime Now() { return Datetime(time(nullptr)); }
  static Datetime FromISO8601(const std::string& str);

  static size_t size() { return sizeof(time_t); }
  const void * Raw() const { return (void*)&time_; }

  std::string ToStrLocalFile() const;
  std::string ToStrLocal() const;
  std::string ToStrUTC() const;

private:
  Datetime(time_t time) : time_(time) {}

  std::string ToStr(struct tm * time_tm, const char * format) const;

  time_t time_;
};

inline int sqlite3_bind_datetime(sqlite3_stmt * stmt, int pos,
    const Datetime& datetime) {
  return sqlite3_bind_blob(stmt, pos, datetime.Raw(), Datetime::size(),
      SQLITE_TRANSIENT);
}

inline Datetime sqlite3_column_datetime(sqlite3_stmt * stmt, int iCol) {
  const void * ptr = sqlite3_column_blob(stmt, iCol);

  if (ptr == nullptr) {
    throw std::runtime_error("Database contains empty Datetime");
  } else {
    if ((size_t)sqlite3_column_bytes(stmt, iCol) != Datetime::size()) {
      throw std::runtime_error("Invalid Datetime in database");
    }
    return Datetime::FromRaw(ptr);
  }
}

#endif // SRC_DATETIME_HPP_
