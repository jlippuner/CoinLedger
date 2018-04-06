/// \file CSV.hpp
/// \author jlippuner
/// \since Apr 05, 2018
///
/// \brief
///
///

#ifndef SRC_IMPORTERS_CSV_HPP_
#define SRC_IMPORTERS_CSV_HPP_

#include <string>
#include <vector>

class CSV {
public:
  CSV(const std::string& path, bool first_line_is_header = true);

  void HandleField(void * str, size_t len);
  void HandleRow(int c);

  bool HasHeader() const { return has_header_; }
  const std::vector<std::string>& Header() const { return header_; }
  const std::vector<std::vector<std::string>> Content() const {
    return content_;
  }

  size_t size() const { return content_.size(); }
  size_t num_fields() const { return num_fields_; }
  const std::vector<std::string>& operator[](size_t i) const {
    return content_[i];
  }

private:
  // true if the first line was considered a header
  bool has_header_;

  // the header, field by field
  std::vector<std::string> header_;

  // the content of the file, line by line, and field by field for each line
  std::vector<std::vector<std::string>> content_;

  size_t num_fields_;

  bool parsing_first_record_;
  std::vector<std::string> current_record_;
};

#endif // SRC_IMPORTERS_CSV_HPP_