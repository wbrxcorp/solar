#!/usr/bin/python
import cgi,cgitb,json,sys,os,copy,time,hmac,hashlib,base64,urllib2,xml.etree.ElementTree
import MySQLdb

if "SCRIPT_FILENAME" in os.environ: cgitb.enable()

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

def create_request_url(access_key,secret_key,associate_tag,operation,args):
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

def parse_products(response):
  tree = xml.etree.ElementTree.fromstring(response)
  products = {}
  ns = {"ecs": "http://webservices.amazon.com/AWSECommerceService/2011-08-01"}
  for item in tree.findall(".//ecs:Item", ns):
    asin = item.find("ecs:ASIN", ns).text
    title = item.find("ecs:ItemAttributes/ecs:Title", ns).text
    price = None
    availability = None
    for offer in item.findall("ecs:Offers/ecs:Offer", ns):
      if offer.find("ecs:OfferAttributes/ecs:Condition", ns).text !='New': continue
      price = int(offer.find("ecs:OfferListing/ecs:Price/ecs:Amount", ns).text)
      availability = offer.find("ecs:OfferListing/ecs:Availability", ns)
      availability = availability.text if availability is not None else offer.find("ecs:OfferListing/ecs:AvailabilityAttributes/ecs:AvailabilityType", ns).text
      break
    if price is None or availability is None: continue
    #else
    products[asin] = {"title":title,"price":price,"availability":availability}

  return products

def update_products(asin_list, system_config):
  products_to_be_checked = []

  with Connection() as cur:
    for asin in amazon:
      cur.execute("select asin from products where asin=%s", (asin,))
      if cur.fetchone() is None: products_to_be_checked.append(asin)
    cur.execute("select asin from products where now()-interval 30 minute < updated_at order by updated_at limit 10")
    for row in cur: products_to_be_checked.append(row[0])

  if len(products_to_be_checked) < 1: return

  args = {"ItemId":",".join(products_to_be_checked[:10]), "ResponseGroup":"ItemAttributes,OfferFull,BrowseNodes"}
  url = create_request_url(system_config["amazon_access_key_id"], system_config["amazon_secret_access_key"], system_config["amazon_associate_tag"], "ItemLookup", args)

  try:
    response = urllib2.urlopen(url).read()
  except urllib2.HTTPError:
    return

  #print response
  products = parse_products(response)

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

  amazon = json.load(open("amazon.json"))

  update_products(amazon, system_config)

  products = {}

  with Connection() as cur:
    cur.execute("select asin,title,price,availability from products")
    for row in cur:
      asin = row[0]
      title = row[1]
      price = row[2]
      availability = row[3]
      products[asin] = {"asin":asin,"title":title,"price":price,"availability":availability}

  amazon_result = []
  for asin in amazon:
    if asin not in products: continue
    amazon_result.append(products[asin])

  print "Content-Type: application/json"
  print
  json.dump({"amazon":amazon_result}, sys.stdout)

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
