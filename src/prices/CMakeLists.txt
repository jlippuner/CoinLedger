set(srcs
  DailyData.cpp
  PriceSource.cpp
)

add_CoinLedger_library(prices "${srcs}")

set(SWIG_deps
  DailyData.hpp
  PriceSource.hpp
)

add_SWIG_dependencies("${SWIG_deps}")