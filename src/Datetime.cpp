/// \file Datetime.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Datetime.hpp"

#include <stdexcept>

Datetime Datetime::FromISO8601(const std::string& str) {
  int year, month, day, hour, minute;
  float second;
  if (sscanf(str.c_str(), "%d-%d-%dT%d:%d:%fZ", &year, &month, &day, &hour,
          &minute, &second) != 6)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as an ISO 8601 date time");

  struct tm utc;
  utc.tm_year = year - 1900;
  utc.tm_mon = month - 1;
  utc.tm_mday = day;
  utc.tm_hour = hour;
  utc.tm_min = minute;
  utc.tm_sec = second;
  utc.tm_isdst = 0;

  // unfortunately, C can only convert a struct tm to time_t by interpreting it
  // as local time
  time_t local = mktime(&utc);

  // find offset between local and UTC
  time_t zero = 0;
  struct tm* utc_tm = gmtime(&zero);
  utc_tm->tm_isdst = 0;
  time_t utc_time = mktime(utc_tm);

  return Datetime(local - utc_time);
}

std::string Datetime::ToStrLocalFile() const {
  struct tm* local_time = localtime(&time_);
  return ToStr(local_time, "%F_%T");
}

std::string Datetime::ToStrLocal() const {
  struct tm* local_time = localtime(&time_);
  return ToStr(local_time, "%F %T");
}

std::string Datetime::ToStrUTC() const {
  struct tm* utc = gmtime(&time_);
  return ToStr(utc, "%F %T");
}

std::string Datetime::ToStr(struct tm* time_tm, const char* format) const {
  char buf[1024];
  strftime(buf, 1024, format, time_tm);
  return std::string(buf);
}
