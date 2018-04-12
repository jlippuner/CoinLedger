/// \file Datetime.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Datetime.hpp"

#include <stdexcept>

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

Datetime Datetime::FromBittrex(const std::string& str) {
  int year, month, day, hour, minute;
  float second;
  char ampm;
  if (sscanf(str.c_str(), "%d/%d/%d %d:%d:%f %cM", &month, &day, &year, &hour,
          &minute, &second, &ampm) != 7)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  if (hour == 12) hour = 0;
  if ((ampm != 'A') && (ampm != 'P'))
    throw std::invalid_argument(
        "Expected AM or PM at the end of '" + str + "'");
  if (ampm == 'P') hour += 12;

  return MakeDatetime(year, month, day, hour, minute, second, true);
}

Datetime Datetime::Parse(const std::string& str, const char* format, bool UTC) {
  int year, month, day, hour, minute;
  float second;
  if (sscanf(str.c_str(), format, &year, &month, &day, &hour, &minute,
          &second) != 6)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  return MakeDatetime(year, month, day, hour, minute, second, UTC);
}

Datetime Datetime::MakeDatetime(int year, int month, int day, int hour,
    int minute, float second, bool UTC) {
  struct tm datetime;
  datetime.tm_year = year - 1900;
  datetime.tm_mon = month - 1;
  datetime.tm_mday = day;
  datetime.tm_hour = hour;
  datetime.tm_min = minute;
  datetime.tm_sec = second;
  if (UTC) datetime.tm_isdst = 0;

  if (UTC) {
    // unfortunately, C can only convert a struct tm to time_t by interpreting
    // it as local time
    time_t local = mktime(&datetime);

    // find offset between local and UTC
    time_t zero = 0;
    struct tm* utc_tm = gmtime(&zero);
    utc_tm->tm_isdst = 0;
    time_t utc_time = mktime(utc_tm);

    return Datetime(local - utc_time);
  } else {
    return Datetime(mktime(&datetime));
  }
}

std::string Datetime::ToStr(struct tm* time_tm, const char* format) const {
  char buf[1024];
  strftime(buf, 1024, format, time_tm);
  return std::string(buf);
}
