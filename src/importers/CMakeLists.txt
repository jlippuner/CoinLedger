set(srcs
  CSV.cpp
  GDAX.cpp
)

add_CoinLedger_library(importers "${srcs}")

set(SWIG_deps
  CSV.hpp
  GDAX.hpp
)

add_SWIG_dependencies("${SWIG_deps}")