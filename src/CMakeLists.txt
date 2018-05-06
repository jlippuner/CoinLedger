add_subdirectory(importers)
add_subdirectory(prices)
add_subdirectory(taxes)

set(srcs
  Account.cpp
  Amount.cpp
  Balance.cpp
  Coin.cpp
  Datetime.cpp
  File.cpp
  Split.cpp
  Transaction.cpp
)

add_CoinLedger_library(src "${srcs}")

set(SWIG_deps
  Account.hpp
  Amount.hpp
  Balance.hpp
  Coin.hpp
  Datetime.hpp
  File.hpp
  Split.hpp
  Transaction.hpp
  UUID.hpp
  CoinLedger.i
)

add_SWIG_dependencies("${SWIG_deps}")
