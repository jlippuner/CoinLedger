#ifndef SRC_PRICESOURCE_HPP_
#define SRC_PRICESOURCE_HPP_

#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>


class PriceSource {
public:
  static PriceSource& Instance() {
    static PriceSource p;
    return p;
  }

  ~PriceSource() {
    curlpp::terminate();
  }

  static std::string GetURL(std::string url) {
    return Instance().DoGetURL(url);
  }

private:
  PriceSource() { 
    curlpp::initialize();
  }

  std::string DoGetURL(std::string url) const;
};

#endif // SRC_PRICESOURCE_HPP_