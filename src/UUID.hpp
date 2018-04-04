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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <sqlite3.h>

class uuid_t {
public:
  uuid_t() : id_(boost::uuids::nil_uuid()) {}
  //uuid_t(const uuid_t& id) : id_(id.id_) {}
  uuid_t(boost::uuids::uuid id) : id_(id) {}

  static uuid_t Nil() { return uuid_t(boost::uuids::nil_uuid()); }
  static uuid_t Random() {
    static boost::uuids::random_generator generator;
    return uuid_t(generator());
  }

  struct hash {
    inline std::size_t operator()(const uuid_t& val) const {
      return boost::uuids::hash_value(val.id_);
    }
  };

  bool operator==(const uuid_t& other) const {
    return id_ == other.id_;
  }

  size_t size() const { return id_.size(); }
  uint8_t * data() { return id_.data; }
  const uint8_t * data() const { return id_.data; }
  bool is_nil() const { return id_.is_nil(); }

  std::string ToString() const {
    std::ostringstream os;
    os << id_;
    return os.str();
  }

  friend std::ostream& operator<<(std::ostream& stm, const uuid_t& id);
private:
  boost::uuids::uuid id_;
};

inline std::ostream& operator<<(std::ostream& stm, const uuid_t& id) {
  stm << id.id_;
  return stm;
}

inline int sqlite3_bind_uuid(sqlite3_stmt * stmt, int pos, const uuid_t& uuid) {
  if (uuid.is_nil()) {
    return sqlite3_bind_null(stmt, pos);
  } else {
    return sqlite3_bind_blob(stmt, pos, uuid.data(), uuid.size(),
        SQLITE_TRANSIENT);
  }
}

inline uuid_t sqlite3_column_uuid(sqlite3_stmt * stmt, int iCol) {
  const void * ptr = sqlite3_column_blob(stmt, iCol);

  if (ptr == nullptr) {
    return uuid_t::Nil();
  } else {
    uuid_t uuid;
    if ((size_t)sqlite3_column_bytes(stmt, iCol) != uuid.size()) {
      throw std::runtime_error("Invalid UUID in database");
    }

    memcpy(uuid.data(), ptr, uuid.size());
    return uuid;
  }
}

#endif // SRC_UUID_HPP_
