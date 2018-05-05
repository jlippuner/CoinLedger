set(srcs
  Basis.cpp
)

add_CoinLedger_library(taxes "${srcs}")

set(SWIG_deps
  Acquisition.hpp
  Basis.hpp
)

add_SWIG_dependencies("${SWIG_deps}")