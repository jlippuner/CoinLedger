set(srcs
  Binance.cpp
  Bittrex.cpp
  CoreWallet.cpp
  CSV.cpp
  ETHAccount.cpp
  ETHECR20Account.cpp
  ElectrumWallet.cpp
  GDAX.cpp
  Kraken.cpp
  MiningPoolHub.cpp
  NiceHash.cpp
  XRPAccount.cpp
)

add_CoinLedger_library(importers "${srcs}")

set(SWIG_deps
  Binance.hpp
  Bittrex.hpp
  CoreWallet.hpp
  CSV.hpp
  ETHAccount.hpp
  ETHECR20Account.hpp
  ElectrumWallet.hpp
  GDAX.hpp
  Kraken.hpp
  MiningPoolHub.hpp
  NiceHash.hpp
  XRPAccount.hpp
)

add_SWIG_dependencies("${SWIG_deps}")