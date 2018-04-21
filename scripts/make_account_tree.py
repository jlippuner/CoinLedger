#!/usr/bin/env python

from CoinLedger import *

f = File.InitNewFile()

assets = f.GetAccount("Assets")

wallets = Account.Create(f, "Wallets", True, assets, False)
Account.Create(f, "Bitcoin Core",       False, wallets, True, f.GetCoinBySymbol("BTC"))
Account.Create(f, "SegWit2x Core",      False, wallets, True, f.GetCoinBySymbol("B2X"))
Account.Create(f, "Dash Core",          False, wallets, True, f.GetCoinBySymbol("DASH"))
Account.Create(f, "Electron Bitcoin Cash", False, wallets, True, f.GetCoinBySymbol("BCH"))
Account.Create(f, "Electrum Litecoin",  False, wallets, True, f.GetCoinBySymbol("LTC"))
Account.Create(f, "Electrum Verge",     False, wallets, True, f.GetCoinBySymbol("XVG"))
Account.Create(f, "Toast Ripple",       False, wallets, True, f.GetCoinBySymbol("XRP"))
Account.Create(f, "Vertcoin Core",      False, wallets, True, f.GetCoinBySymbol("VTC"))
Account.Create(f, "Vivo Core",          False, wallets, True, f.GetCoinBySymbol("VIVO"))
Account.Create(f, "Ethereum Wallet",    False, wallets, True, f.GetCoinBySymbol("ETH"))
Account.Create(f, "Exodus Wallet",      False, wallets, False)
#Account.Create(f, "", False, wallets, True, f.GetCoinBySymbol(""))

exchanges = Account.Create(f, "Exchanges", True, assets, False)
Account.Create(f, "GDAX",     False, exchanges, False)
Account.Create(f, "Kraken",   False, exchanges, False)
Account.Create(f, "Bittrex",  False, exchanges, False)
Account.Create(f, "Binance",  False, exchanges, False)
#Account.Create(f, "", False, exchanges, False)

pools = Account.Create(f, "Mining Pools", True, assets, False)
Account.Create(f, "MiningPoolHub.com", False, pools, False)
Account.Create(f, "NiceHash.com", False, pools, True, f.GetCoinBySymbol("BTC"))
#Account.Create(f, "", False, pools, False)


income = f.GetAccount("Income")

forks = Account.Create(f, "Hard Forks", False, income, False)
Account.Create(f, "Bitcoin", False, forks, False)

mining = Account.Create(f, "Mining", False, income, False)
Account.Create(f, "MiningPoolHub.com", False, mining, False)
Account.Create(f, "zpool.ca", False, mining, False)
Account.Create(f, "NiceHash.com", False, mining, True, f.GetCoinBySymbol("BTC"))
Account.Create(f, "ethermine.org", False, mining, True, f.GetCoinBySymbol("ETH"))
#Account.Create(f, "", False, mining, False)

equity = f.GetAccount("Equity")
Account.Create(f, "USD Investment", False, equity, True, f.GetCoinBySymbol("USD"))

expenses = f.GetAccount("Expenses")

Account.Create(f, "Mining Equipment", False, expenses, False)

icos = Account.Create(f, "Initial Coin Offerings", True, expenses, False)
Account.Create(f, "Exscudo", False, icos, False)

transaction_fees = Account.Create(f, "Transaction Fees", False, expenses, False)
Account.Create(f, "Kraken Withdrawals", False, transaction_fees, False)

trading_fees = Account.Create(f, "Trading Fees", True, expenses, False)
Account.Create(f, "GDAX",     False, trading_fees, False)
Account.Create(f, "Kraken",   False, trading_fees, False)
Account.Create(f, "Bittrex",  False, trading_fees, False)
Account.Create(f, "Binance",  False, trading_fees, False)

mining_fees = Account.Create(f, "Mining Fees", True, expenses, False)
Account.Create(f, "MiningPoolHub.com", False, mining_fees, False)

f.Save("default.sqlite3")

