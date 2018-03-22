add_executable(exec test.cpp)
target_link_libraries(exec 
  CoinLedger_static
  ${COINLEDGER_EXTERNAL_LIBS}
)