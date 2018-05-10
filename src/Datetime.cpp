/// \file Datetime.cpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#include "Datetime.hpp"

#include <stdexcept>

Datetime Datetime::FromBittrex(const std::string& str) {
  int year, month, day, hour, minute;
  float second;
  char ampm;
  if (sscanf(str.c_str(), "%2d/%2d/%4d %2d:%2d:%f %cM", &month, &day, &year,
          &hour, &minute, &second, &ampm) != 7)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  if (hour == 12) hour = 0;
  if ((ampm != 'A') && (ampm != 'P'))
    throw std::invalid_argument(
        "Expected AM or PM at the end of '" + str + "'");
  if (ampm == 'P') hour += 12;

  return MakeDatetime(year, month, day, hour, minute, second, true);
}

std::string Datetime::ToStrLocalFile() const {
  struct tm* local_time = localtime(&time_);
  return ToStr(local_time, "%F_%T");
}

// std::string Datetime::ToStrLocal() const {
//   struct tm* local_time = localtime(&time_);
//   return ToStr(local_time, "%F %T");
// }

std::string Datetime::ToStrUTC() const {
  struct tm* utc = gmtime(&time_);
  return ToStr(utc, "%F %T");
}

std::string Datetime::ToStrDayUTC() const {
  struct tm* utc = gmtime(&time_);
  return ToStr(utc, "%F");
}

Datetime Datetime::EndOfDay() const {
  // if we don't want to account for leap seconds, we could just round down the
  // UNIX timestamp to a multiple of 86400, but we'll use struct tm instead to
  // account for leap seconds
  struct tm* utc = gmtime(&time_);
  return MakeDatetime(
      utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday, 23, 59, 59, true);
}

int64_t Datetime::DailyDataDayFromStr(std::string str) {
  char month_char[4];
  int day, year;
  if (sscanf(str.c_str(), "%3s %2d, %4d", month_char, &day, &year) != 3)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  std::string month_str(month_char);
  int month = 0;
  if (month_str == "Jan")
    month = 1;
  else if (month_str == "Feb")
    month = 2;
  else if (month_str == "Mar")
    month = 3;
  else if (month_str == "Apr")
    month = 4;
  else if (month_str == "May")
    month = 5;
  else if (month_str == "Jun")
    month = 6;
  else if (month_str == "Jul")
    month = 7;
  else if (month_str == "Aug")
    month = 8;
  else if (month_str == "Sep")
    month = 9;
  else if (month_str == "Oct")
    month = 10;
  else if (month_str == "Nov")
    month = 11;
  else if (month_str == "Dec")
    month = 12;
  else
    throw std::invalid_argument("Invalid month string '" + month_str + "'");

  auto date = MakeDatetime(year, month, day, 12, 0, 0, true);
  return date.DailyDataDay();
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

std::string Datetime::ToStr(struct tm* time_tm, const char* format) {
  char buf[1024];
  strftime(buf, 1024, format, time_tm);
  return std::string(buf);
}
