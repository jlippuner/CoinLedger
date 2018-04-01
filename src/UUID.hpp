/// \file UUID.hpp
/// \author jlippuner
/// \since Mar 22, 2018
///
/// \brief
///
///

#ifndef SRC_UUID_HPP_
#define SRC_UUID_HPP_

#include <cstring>
#include <stdexcept>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <sqlite3.h>

using uuid_t = boost::uuids::uuid;

inline int sqlite3_bind_uuid(sqlite3_stmt * stmt, int pos, const uuid_t& uuid) {
  if (uuid.is_nil()) {
    return sqlite3_bind_null(stmt, pos);
  } else {
    return sqlite3_bind_blob(stmt, pos, uuid.data, uuid.size(),
        SQLITE_TRANSIENT);
  }
}

inline uuid_t sqlite3_column_uuid(sqlite3_stmt * stmt, int iCol) {
  const void * ptr = sqlite3_column_blob(stmt, iCol);

  if (ptr == nullptr) {
    return boost::uuids::nil_uuid();
  } else {
    uuid_t uuid;
    if (sqlite3_column_bytes(stmt, iCol) != uuid.size()) {
      throw std::runtime_error("Invalid UUID in database");
    }

    memcpy(uuid.data, ptr, uuid.size());
    return uuid;
  }
}

#endif // SRC_UUID_HPP_
