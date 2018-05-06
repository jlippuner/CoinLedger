set(srcs
  Candles.cpp
  H5Helper.cpp
  PriceSource.cpp
)

add_CoinLedger_library(prices "${srcs}")

set(SWIG_deps
  Candles.hpp
  H5Helper.hpp
  PriceSource.hpp
)

add_SWIG_dependencies("${SWIG_deps}")