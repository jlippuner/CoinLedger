#!/usr/bin/env python

from CoinLedger import *

f = File.Open("import.sqlite3")

assets = f.GetAccount("Assets")
mining_fees = f.GetAccount("Expenses::Mining Fees")
trading_fees = f.GetAccount("Expenses::Trading Fees")
txn_fees = f.GetAccount("Expenses::Transaction Fees")
income_forks = f.GetAccount("Income::Hard Forks")
income_mining = f.GetAccount("Income::Mining")

until = Datetime.Now()

tax = Taxes(f, until, assets, mining_fees, trading_fees,
            txn_fees, income_forks, income_mining)

tax.PrintMiningIncome(f)
