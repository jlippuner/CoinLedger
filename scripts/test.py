#!/usr/bin/env python

from CoinLedger import *

f = File.Open("import.sqlite3")
f.PrintAccountBalances()
f.PrintUnbalancedTransactions()

btc = f.GetCoinBySymbol("BTC")
