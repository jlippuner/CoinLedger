/// \file CSV.cpp
/// \author jlippuner
/// \since Apr 05, 2018
///
/// \brief
///
///

#include "CSV.hpp"

#include <stdexcept>

#include <csv.h>

namespace {

void handle_field(void * str, size_t len, void * csv) {
  ((CSV*)csv)->HandleField(str, len);
}

void handle_row(int c, void * csv) {
  ((CSV*)csv)->HandleRow(c);
}

}

CSV::CSV(const std::string& path, bool first_line_is_header) :
    has_header_(first_line_is_header) {
  // create the parser
  struct csv_parser parser;
  if (csv_init(&parser, CSV_STRICT | CSV_STRICT_FINI | CSV_APPEND_NULL) != 0) {
    throw std::runtime_error("Could not initialize the CSV parser");
  }

  // open the file
  FILE * f = fopen(path.c_str(), "rb");
  if (f == nullptr) {
    throw std::runtime_error("Could not open file '" + path + "' for reading");
  }

  char buf[1024];
  size_t num_read = fread(buf, 1, 1024, f);
  parsing_first_record_ = true;
  header_.clear();
  content_.clear();
  current_record_.clear();

  while (num_read > 0) {
    if (csv_parse(&parser, buf, num_read, handle_field, handle_row, this)
        != num_read) {
      throw std::runtime_error("Parsing CSV file '" + path + "' failed");
    }

    num_read = fread(buf, 1, 1024, f);
  }

  if (csv_fini(&parser, handle_field, handle_row, this) != 0)
    throw std::runtime_error("Parsing CSV file '" + path + "' failed");

  fclose(f);
  csv_free(&parser);
}

void CSV::HandleField(void * str, size_t /*len*/) {
  current_record_.push_back(std::string((char*)str));
}

void CSV::HandleRow(int /*c*/) {
  if (parsing_first_record_) {
    num_fields_ = current_record_.size();

    if (has_header_)
      header_ = current_record_;
    else
      content_.push_back(current_record_);

    parsing_first_record_ = false;
  } else {
    if (current_record_.size() != num_fields_) {
      throw std::runtime_error("Got a CSV record with a different number of "
          "fields");
    }
    content_.push_back(current_record_);
  }

  current_record_.clear();
  current_record_.reserve(num_fields_);
}
