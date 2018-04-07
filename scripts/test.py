#!/usr/bin/env python

from CoinLedger import *

f = File.Open("test.sqlite3")

c = CSV("/home/jlippuner/MEGA/finances/crypto/GDAX_USD_account.csv")
d = Datetime.FromISO8601("2017-06-21T21:39:19.192Z")
d.ToStrUTC()
