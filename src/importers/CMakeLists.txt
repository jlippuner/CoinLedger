set(srcs
  Binance.cpp
  Bittrex.cpp
  CoreWallet.cpp
  CSV.cpp
  ElectrumWallet.cpp
  GDAX.cpp
  Kraken.cpp
  XRPAccount.cpp
)

add_CoinLedger_library(importers "${srcs}")

set(SWIG_deps
  Binance.hpp
  Bittrex.hpp
  CoreWallet.hpp
  CSV.hpp
  ElectrumWallet.hpp
  GDAX.hpp
  Kraken.hpp
  XRPAccount.hpp
)

add_SWIG_dependencies("${SWIG_deps}")