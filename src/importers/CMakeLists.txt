set(srcs
  Bittrex.cpp
  CSV.cpp
  GDAX.cpp
  Kraken.cpp
)

add_CoinLedger_library(importers "${srcs}")

set(SWIG_deps
  Bittrex.hpp
  CSV.hpp
  GDAX.hpp
  Kraken.hpp
)

add_SWIG_dependencies("${SWIG_deps}")