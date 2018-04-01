/// \file UUID.hpp
/// \author jlippuner
/// \since Mar 22, 2018
///
/// \brief
///
///

#ifndef SRC_UUID_HPP_
#define SRC_UUID_HPP_

#include <vector>

#include <boost/uuid/uuid.hpp>
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

#endif // SRC_UUID_HPP_
