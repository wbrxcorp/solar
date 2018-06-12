#!/usr/bin/python
import os,uuid,json,time,sys,copy,hmac,hashlib,base64,urllib2,xml.etree.ElementTree
import MySQLdb
import restkit

class Connection:
    def __init__(self, dbhost = "localhost", dbname = "coins", autocommit = True):
        self.dbhost = dbhost
        self.dbname = dbname
        self.autocommit = autocommit

    def __enter__(self):
        self.conn = MySQLdb.connect(host=self.dbhost,db=self.dbname)
        self.conn.autocommit(self.autocommit)
        self.cursor = self.conn.cursor()
        return self.cursor

    def __exit__(self, exc_type, exc_value, traceback):
        self.cursor.close()
        self.conn.close()

def write_file_atomic(filename, content):
    tmp_filename = os.path.join(os.path.dirname(os.path.abspath(filename)), str(uuid.uuid4()) + ".tmp")
    with open(tmp_filename, "w") as f:
        f.write(content)
    os.rename(tmp_filename, filename)

def should_download_new_file(filename, ttl_minutes = 60, self_filename = __file__):
    if not os.path.exists(filename): return True
    mtime = os.path.getmtime(filename)
    if os.path.getmtime(self_filename) > mtime: return True
    return (time.time() - mtime) / 60 >= ttl_minutes

def get_daily_earnings(coin_id, hashrate):
    prices = json.load(open("%s.prices.json" % coin_id))
    return (prices["data"]["daily_dollars_per_hashrate"] * hashrate, prices["data"]["daily_coins_per_hashrate"] * hashrate)

def ecs_create_request_url(access_key,secret_key,associate_tag,operation,args):
  _args = copy.copy(args)
  _args["AWSAccessKeyId"] = access_key
  _args["AssociateTag"] = associate_tag
  _args["Service"] = "AWSECommerceService"
  _args["Timestamp"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
  _args["Operation"] = operation
  query_string = ""
  for arg in sorted(_args.items(), key=lambda x: x[0]):
    if len(query_string) > 0: query_string += "&"
    query_string += "%s=%s" % (arg[0], urllib2.quote(arg[1]))

  string_so_sign = "GET\necs.amazonaws.jp\n/onca/xml\n%s" % query_string
  signature = urllib2.quote(base64.b64encode(hmac.new(secret_key, string_so_sign, hashlib.sha256).digest()))
  return "http://ecs.amazonaws.jp/onca/xml?%s&Signature=%s" % (query_string, signature)

def ecs_parse_products(response):
  tree = xml.etree.ElementTree.fromstring(response)
  products = {}
  ns = {"ecs": "http://webservices.amazon.com/AWSECommerceService/2011-08-01"}
  for item in tree.findall(".//ecs:Item", ns):
    asin = item.find("ecs:ASIN", ns).text
    title = item.find("ecs:ItemAttributes/ecs:Title", ns).text
    price = item.find("ecs:OfferSummary/ecs:LowestNewPrice/ecs:Amount", ns)
    if price is None: continue
    #else
    price = int(price.text)
    availability = "-"
    for offer in item.findall("ecs:Offers/ecs:Offer", ns):
      if offer.find("ecs:OfferAttributes/ecs:Condition", ns).text !='New': continue
      price = int(offer.find("ecs:OfferListing/ecs:Price/ecs:Amount", ns).text)
      availability = offer.find("ecs:OfferListing/ecs:Availability", ns)
      availability = availability.text if availability is not None else offer.find("ecs:OfferListing/ecs:AvailabilityAttributes/ecs:AvailabilityType", ns).text
      break
    products[asin] = {"title":title,"price":price,"availability":availability}

  return products

def amazon_update_products(asin_list, system_config):
  products_to_be_checked = []

  with Connection() as cur:
    for asin in amazon:
      cur.execute("select asin from products where asin=%s", (asin,))
      if cur.fetchone() is None: products_to_be_checked.append(asin)
    cur.execute("select asin from products where now()-interval 30 minute > updated_at order by updated_at limit 10")
    for row in cur: products_to_be_checked.append(row[0])

  if len(products_to_be_checked) < 1: return

  args = {"ItemId":",".join(products_to_be_checked[:10]), "ResponseGroup":"ItemAttributes,OfferFull,BrowseNodes"}
  url = ecs_create_request_url(system_config["amazon_access_key_id"], system_config["amazon_secret_access_key"], system_config["amazon_associate_tag"], "ItemLookup", args)

  try:
      response = urllib2.urlopen(url).read()
  except urllib2.HTTPError:
    return

  #print response
  products = ecs_parse_products(response)

  with Connection() as cur:
    for asin,product in products.iteritems():
      title = product["title"]
      price = product["price"]
      availability = product["availability"]
      cur.execute("insert into products(asin,title,price,availability,created_at,updated_at) values(%s,%s,%s,%s,now(),now()) on duplicate key update title=%s,price=%s,availability=%s,updated_at=now()", (asin,title,price,availability,title,price,availability))

if __name__ == '__main__':
    system_config = {}

    with Connection() as cur:
        cur.execute("select `key`,`value` from system_config")
        for row in cur:
            system_config[row[0]] = row[1]

    filename = "usd.json"
    if should_download_new_file(filename):
        r = restkit.request("http://api.fixer.io/latest?base=USD")
        if r.status_int == 200: write_file_atomic(filename, r.body_string())

    filename = "btc.json"
    if should_download_new_file(filename, 10):
        r = restkit.request("https://api.coindesk.com/v1/bpi/currentprice.json")
        if r.status_int == 200: write_file_atomic(filename, r.body_string())

    coins = json.load(open("coins.json"))
    workers = {}

    for coin in coins:
        coin_id = coin["id"]

        filename = "%s.prices.json" % coin_id
        if should_download_new_file(filename, 10):
            r1 = restkit.request("https://api.nanopool.org/v1/%s/prices" % coin_id)
            if r1.status_int == 200:
                prices = json.loads(r1.body_string())
                if prices["status"]:
                    r2 = restkit.request("https://api.nanopool.org/v1/%s/approximated_earnings/1.0" % coin_id)
                    if r2.status_int == 200:
                        earnings = json.loads(r2.body_string())
                        if earnings["status"]:
                            prices["data"]["daily_dollars_per_hashrate"] = earnings["data"]["day"]["dollars"]
                            prices["data"]["daily_coins_per_hashrate"] = earnings["data"]["day"]["coins"]
                            write_file_atomic(filename, json.dumps(prices))

        if "pool" in coin and coin["pool"] == "mph":
            user_id = coin["mph_user_id"]
            api_key = coin["mph_api_key"]
            mph_coin_id = coin["mph_coin_id"]

            filename = "%s.user.json" % coin_id
            if should_download_new_file(filename):
                r1 = restkit.request("http://%s.miningpoolhub.com/index.php?page=api&action=getuserhashrate&api_key=%s&id=%s" % (mph_coin_id, api_key, user_id))
                r2 = restkit.request("http://%s.miningpoolhub.com/index.php?page=api&action=getuserbalance&api_key=%s&id=%s" % (mph_coin_id, api_key, user_id))
                if r1.status_int == 200 and r2.status_int == 200:
                    hashrate = json.loads(r1.body_string())
                    balance = json.loads(r2.body_string())
                    if "getuserhashrate" in hashrate and "data" in hashrate["getuserhashrate"] and "getuserbalance" in balance and "data" in balance["getuserbalance"]:
                        h6 = float(hashrate["getuserhashrate"]["data"] * 1000)
                        user = {"status":True, "data":{"balance":balance["getuserbalance"]["data"]["confirmed"],"avgHashrate":{"h6":h6}}}
                        user["data"]["daily_dollars"], user["data"]["daily_coins"] = get_daily_earnings(coin_id, h6)
                        write_file_atomic(filename, json.dumps(user))
            # todo: workers
            #if os.path.isfile(filename):
            #    user = json.load(open(filename))
            #    for worker in user["data"]["workers"]:
            #        worker_id = worker["id"]
            #        h6 = float(worker["h6"])
            #        if worker_id not in workers: workers[worker_id] = []
            #        workers[worker_id].append({"id":coin_id, "h6":h6})

            filename = "%s.settings.json" % coin_id
            if should_download_new_file(filename):
                write_file_atomic(filename, json.dumps({"status":True,"data":{"payout":None}}))
        else: # nanopool
            address = coin["address"]

            filename = "%s.user.json" % coin_id
            if should_download_new_file(filename):
                r = restkit.request("https://api.nanopool.org/v1/%s/user/%s" % (coin_id, address))
                if r.status_int == 200: write_file_atomic(filename, r.body_string())

            user = json.load(open(filename)) if os.path.isfile(filename) else None

            if user is not None and user["status"]:
                h6 = float(user["data"]["avgHashrate"]["h6"])
                user["data"]["daily_dollars"], user["data"]["daily_coins"] = get_daily_earnings(coin_id, h6)
                write_file_atomic("%s.json" % coin_id, json.dumps(user))

                for worker in user["data"]["workers"]:
                    worker_id = worker["id"]
                    h6 = float(worker["h6"])
                    if worker_id not in workers: workers[worker_id] = []
                    workers[worker_id].append({"id":coin_id, "h6":h6})

            filename = "%s.settings.json" % coin_id
            if should_download_new_file(filename):
                r = restkit.request("https://api.nanopool.org/v1/%s/usersettings/%s" % (coin_id, address))
                if r.status_int == 200:
                    user = json.loads(r.body_string())
                    if user["status"]: write_file_atomic(filename, json.dumps(user))

    workers_list = []
    for worker_id,coins in workers.iteritems():
        total_daily_dollars = 0.0
        for coin in coins:
            coin_id, h6 = coin["id"], coin["h6"]
            if h6 < 0.000001: continue
            coin["daily_dollars"],coin["daily_coins"] = get_daily_earnings(coin_id, h6)
            total_daily_dollars += coin["daily_dollars"]
        workers_list.append({"id":worker_id, "coins":sorted([coin for coin in coins if "daily_dollars" in coin], key=lambda x:-x["daily_dollars"]), "daily_dollars":total_daily_dollars})

    write_file_atomic("workers.json", json.dumps(sorted(workers_list, key=lambda x:-x["daily_dollars"])))

    # update amazon products
    amazon = json.load(open("amazon.json"))
    amazon_update_products(amazon, system_config)
    products = {}

    with Connection() as cur:
        cur.execute("select asin,title,price,availability from products")
        for row in cur:
            asin, title, price, availability = (row[0], row[1], row[2], row[3])
            products[asin] = {"asin":asin,"title":title,"price":price,"availability":availability}

    amazon_result = []
    for asin in amazon:
        if asin not in products: continue
        amazon_result.append(products[asin])

    write_file_atomic("products.json", json.dumps(amazon_result))

"""
create table system_config(`key` varchar(32) primary key,`value` varchar(128));
insert into system_config(`key`,`value`) values('amazon_access_key_id','YOUR_ACCESS_KEY_HERE');
insert into system_config(`key`,`value`) values('amazon_secret_access_key','YOUR_SECRET_ACCESS_KEY_HERE');
insert into system_config(`key`,`value`) values('amazon_associate_tag','YOUR_ASSOCIATE_TAG_HERE');

create table products (
    asin varchar(32) not null primary key,
    title varchar(256) not null,
    price int not null,
    availability varchar(64),
    created_at timestamp not null default current_timestamp,
    updated_at timestamp not null default current_timestamp
);
"""
