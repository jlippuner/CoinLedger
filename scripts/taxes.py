#!/usr/bin/env python

from CoinLedger import *

#f = File.Open("import.sqlite3")
f = File.Open("import_with_price_data.sqlite3")

assets = f.GetAccount("Assets")
exchanges = f.GetAccount("Assets::Exchanges")
equity = f.GetAccount("Equity")
expenses = f.GetAccount("Expenses")
mining_fees = f.GetAccount("Expenses::Mining Fees")
trading_fees = f.GetAccount("Expenses::Trading Fees")
txn_fees = f.GetAccount("Expenses::Transaction Fees")
income_forks = f.GetAccount("Income::Hard Forks")
income_mining = f.GetAccount("Income::Mining")

until = Datetime.Now()

tax = Taxes(f, until, assets, exchanges, equity, expenses, mining_fees,
            trading_fees, txn_fees, income_forks, income_mining)

tax.PrintIncome(f)
tax.PrintSpending(f)
tax.PrintCapitalGainsLosses(f, 365, False)

#f.Save("import_with_price_data.sqlite3")
