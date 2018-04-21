#!/usr/bin/env python3

import csv
import sys
import datetime
import requests
import urllib3
urllib3.disable_warnings()

if (len(sys.argv) != 2):
  print("Usage: %s <csv file without timestamps>" % sys.argv[0])
  sys.exit(1)

with open(sys.argv[1]) as inp:
  reader = csv.reader(inp)
  first = True

  print("transaction_hash,label,confirmations,value,timestamp")

  for row in reader:
    if first:
      if (not row == ['transaction_hash', 'label', 'confirmations', 'value', 'timestamp']):
        print("Unexpected CSV header:")
        print(row)
        sys.exit(1)
      else:
        first = False
    else:
      desc = row[1].replace('"', '""')

      if (row[2] == "-1"):
        # get the number of confirmations and the time stamp
        url = "https://verge-blockchain.info/api/getrawtransaction?txid=%s&decrypt=1" % row[0]
        r = requests.get(url, verify=False)
        res = r.json()
        conf = res["confirmations"]
        time = datetime.datetime.fromtimestamp(int(res["time"])).strftime('%Y-%m-%d %H:%M')
      else:
        conf = int(row[2])
        time = row[4]

      print("%s,\"%s\",%li,%s,%s" % (row[0], desc, conf, row[3], time))
