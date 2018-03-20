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

// Represents a point in time. The resolution is seconds and internally the date
// and time are stored as a unix timestamp in UTC

class Datetime {
public:
  Datetime() = delete;

private:
  time_t time_;
};

#endif // SRC_DATETIME_HPP_
