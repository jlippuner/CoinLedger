#include "PriceSource.hpp"

#include <sstream>

std::string PriceSource::DoGetURL(std::string url) const {
  curlpp::Easy req;
  req.setOpt(curlpp::options::Url(url));
  
  std::ostringstream os;
  req.setOpt(curlpp::options::WriteStream(&os));

  req.perform();
  return os.str();
}
