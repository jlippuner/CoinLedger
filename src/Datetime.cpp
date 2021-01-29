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

Datetime Datetime::FromCelsius(const std::string& str) {
  int year, day, hour, minute;
  char month_char[20];
  char ampm;
  if (sscanf(str.c_str(), "%s %d, %4d %d:%2d %cM", month_char, &day, &year,
          &hour, &minute, &ampm) != 6)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  if (hour == 12) hour = 0;
  if ((ampm != 'A') && (ampm != 'P'))
    throw std::invalid_argument(
        "Expected AM or PM at the end of '" + str + "'");
  if (ampm == 'P') hour += 12;

  auto month = GetMonth(month_char);
  return MakeDatetime(year, month, day, hour, minute, 0, true);
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

std::string Datetime::ToStrDayUTCIRS() const {
  struct tm* utc = gmtime(&time_);
  return ToStr(utc, "%m/%d/%Y");
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
  int day, month, year;
  if (sscanf(str.c_str(), "%4d-%2d-%2dT23:59:59.999Z", &year, &month, &day) != 3)
    throw std::invalid_argument(
        "Cannot parse '" + str + "' as a date and time");

  auto date = MakeDatetime(year, month, day, 12, 0, 0, true);
  return date.DailyDataDay();
}

int Datetime::GetMonth(const char* str) {
  std::string month_str(str);
  if ((month_str == "Jan") || (month_str == "January")) return 1;
  if ((month_str == "Feb") || (month_str == "February")) return 2;
  if ((month_str == "Mar") || (month_str == "March")) return 3;
  if ((month_str == "Apr") || (month_str == "April")) return 4;
  if ((month_str == "May") || (month_str == "May")) return 5;
  if ((month_str == "Jun") || (month_str == "June")) return 6;
  if ((month_str == "Jul") || (month_str == "July")) return 7;
  if ((month_str == "Aug") || (month_str == "August")) return 8;
  if ((month_str == "Sep") || (month_str == "September")) return 9;
  if ((month_str == "Oct") || (month_str == "October")) return 10;
  if ((month_str == "Nov") || (month_str == "November")) return 11;
  if ((month_str == "Dec") || (month_str == "December")) return 12;

  throw std::invalid_argument("Invalid month string '" + month_str + "'");
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

Datetime Datetime::MakeDatetime(struct tm tm_time, bool UTC) {
  if (UTC) tm_time.tm_isdst = 0;

  if (UTC) {
    // unfortunately, C can only convert a struct tm to time_t by interpreting
    // it as local time
    time_t local = mktime(&tm_time);

    // find offset between local and UTC
    time_t zero = 0;
    struct tm* utc_tm = gmtime(&zero);
    utc_tm->tm_isdst = 0;
    time_t utc_time = mktime(utc_tm);

    return Datetime(local - utc_time);
  } else {
    return Datetime(mktime(&tm_time));
  }
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

  return MakeDatetime(datetime, UTC);
}

std::string Datetime::ToStr(struct tm* time_tm, const char* format) {
  char buf[1024];
  strftime(buf, 1024, format, time_tm);
  return std::string(buf);
}
