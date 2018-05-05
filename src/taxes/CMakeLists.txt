set(srcs
  Inventory.cpp
  Taxes.cpp
)

add_CoinLedger_library(taxes "${srcs}")

set(SWIG_deps
  Inventory.hpp
  Taxes.hpp
)

add_SWIG_dependencies("${SWIG_deps}")