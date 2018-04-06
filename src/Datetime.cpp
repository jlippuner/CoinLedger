/// \file Datetime.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Datetime.hpp"

std::string Datetime::ToStrLocalTimeFile() const {
  struct tm * local_time;
  local_time = localtime(&time_);

  char buf[1024];
  strftime(buf, 1024, "%F_%T", local_time);
  return std::string(buf);
}
