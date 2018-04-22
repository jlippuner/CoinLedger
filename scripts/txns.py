#!/usr/bin/env python

import sys
from CoinLedger import *

f = File.Open(sys.argv[1])
f.PrintTransactions()
