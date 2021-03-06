#!/usr/bin/env python

from CoinLedger import *

f = File.Open("import.sqlite3")

assets = f.GetAccount("Assets")
wallets = f.GetAccount("Assets::Wallets")
exchanges = f.GetAccount("Assets::Exchanges")
equity = f.GetAccount("Equity")
expenses = f.GetAccount("Expenses")
mining_fees = f.GetAccount("Expenses::Mining Fees")
trading_fees = f.GetAccount("Expenses::Trading Fees")
txn_fees = f.GetAccount("Expenses::Transaction Fees")
income_forks = f.GetAccount("Income::Hard Forks")
income_mining = f.GetAccount("Income::Mining")

#until = Datetime.Now()
start = Datetime.FromUTC("2018-01-01 00:00:00")
until = Datetime.FromUTC("2019-01-01 00:00:00")

tax = Taxes(f, until, assets, wallets, exchanges, equity, expenses, mining_fees,
            trading_fees, txn_fees, income_forks, income_mining)

tax.PrintIncome(f, start)
tax.PrintSpending(f, start)
tax.PrintCapitalGainsLosses(f, 365, False, start)

f.Save("import_start.sqlite3")
