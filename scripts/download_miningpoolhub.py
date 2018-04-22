#!/usr/bin/env python3

from bs4 import BeautifulSoup
import requests as reqs
import sys
import cfscrape
import time
from http.cookies import SimpleCookie
import urllib3
urllib3.disable_warnings()

def get_html(s, url):
  try:
    time.sleep(2) # needed so that the rate limit won't be exceeded
    #resp = s.get(url, verify=False)
    resp = s.get(url)
    html = resp.text
    return BeautifulSoup(html, "html.parser")
  except OSError:
    print("***** fetching %s failed, retry" % url)
    return get_html(s, url)

def login(ext_cookie):
  s = reqs.Session()

  #s.proxies.update({"http" : "localhost:8080", "https" : "localhost:8080"})

  cookie = SimpleCookie()
  cookie.load(ext_cookie)

  # Even though SimpleCookie is dictionary-like, it internally uses a Morsel object
  # which is incompatible with requests. Manually construct a dictionary instead.
  cs = {}
  for key, morsel in cookie.items():
    cs[key] = morsel.value

  reqs.utils.add_dict_to_cookiejar(s.cookies, cs)

  cfs = cfscrape.create_scraper(sess=s)
  #r = cfs.get("https://miningpoolhub.com/index.php", verify=False)
  r = cfs.get("https://miningpoolhub.com/index.php")

  if (not r.ok):
    print("Login failed")
    sys.exit(1)

  return cfs


# return <number of transactions process>, <encountered last_id true/false>
def download_transaction_page(s, coin, start, last_id, print_head):
  url = "https://%s.miningpoolhub.com/index.php?page=account&action=transactions&start=%i" % (coin, start)
  html = get_html(s, url)

  # find the table with the transactions
  table = None
  for a in html.find_all("article"):
    title = a.select("header > h3")[0].text
    if title == "Transaction History":
      table = a.select("table > tbody")[0]

      if print_head:
        head = a.select("table > thead > tr")[0]
        ths = head("th")
        heads = [th.text.strip() for th in ths]
        print(",".join(heads))

      break

  if table == None:
    print("Couldn't find a transaction history table at %s" % url)
    sys.exit(1)

  num = 0
  for tr in table("tr"):
    tds = tr("td")

    id = tds[0].text

    if (id == last_id):
      return num, True

    date = tds[1].text
    type = tds[2].text
    conf = tds[3].text.strip()
    addr = tds[4].text.strip()

    if addr.startswith("(Shrunk accumulated Credit"):
      num += 1
      continue

    if (len(addr) > 0):
      # extract account address
      if (len(tds[4]("a")) > 0):
        alert = tds[4]("a")[0]["onclick"]
        addr = alert.split("'")[1]

    txn = tds[5].text.strip()
    if (len(txn) > 0):
      txn = tds[5]("a")[0]["title"]

    block = tds[6].text

    amount = tds[7].text.strip()
    if (tds[7]("font")[0]["color"] == "red"):
      amount = "-" + amount

    print("%s,%s,%s,%s,%s,%s,%s,%s" % (id, date, type, conf, addr, txn, block, amount))
    num += 1

  # we didn't encounter the last id
  return num, False


def download_transactions(s, coin, last_id):
  first = True
  start = 0

  while True:
    num, done = download_transaction_page(s, coin, start, last_id, first)
    first = False
    start += num
    if done or (num == 0):
      break

if __name__ == '__main__':
  if (len(sys.argv) != 4):
    print("Usage: %s <coin> <last id> <cookie>" % sys.argv[0])
    print("")
    print("This script cannot automatically log into miningpoolhub.com, because")
    print("of the Google reCAPTCHA. One has to login with a browser and then copy")
    print("the resulting session cookie. Copy the cookie header from the request")
    print("that the authenticated browser makes. The header looks something like")
    print("this: 'Cookie: __cfduid=<...>; PHPSESSID=<...>; cf_clearance=<...>'")
    print("The third argument to this script is the PART AFTER 'Cookie:'. Just")
    print("copy and paste it (and put it inside quotes on the command line).")
    print("Use, for example, the developer tools in Chrome to see the request")
    print("headers with the cookie.")
    sys.exit(1)

  coin = sys.argv[1]
  last_id = sys.argv[2]
  cookie = sys.argv[3]

  s = login(cookie)
  download_transactions(s, coin, last_id)
