add_subdirectory(importers)

set(srcs
  Account.cpp
  Coin.cpp
  Datetime.cpp
  File.cpp
  PriceSource.cpp
  Split.cpp
  Transaction.cpp
)

add_CoinLedger_library(src "${srcs}")

set(SWIG_deps
  Account.hpp
  Amount.hpp
  Coin.hpp
  Datetime.hpp
  File.hpp
  PriceSource.hpp
  Split.hpp
  Transaction.hpp
  UUID.hpp
  CoinLedger.i
)

add_SWIG_dependencies("${SWIG_deps}")
