#!/usr/bin/python
import os,uuid,json
import restkit

def write_file_atomic(filename, content):
    tmp_filename = os.path.join(os.path.dirname(os.path.abspath(filename)), str(uuid.uuid4()) + ".tmp")
    with open(tmp_filename, "w") as f:
        f.write(content)
    os.rename(tmp_filename, filename)

if __name__ == '__main__':
    r = restkit.request("https://api.fixer.io/latest?base=USD")
    if r.status_int == 200: write_file_atomic("usd.json", r.body_string())

    r = restkit.request("https://api.coindesk.com/v1/bpi/currentprice.json")
    if r.status_int == 200: write_file_atomic("btc.json", r.body_string())

    coins = json.load(open("coins.json"))

    for coin in coins:
        coin_id = coin["id"]
        r = restkit.request("https://api.nanopool.org/v1/%s/prices" % coin_id)
        if r.status_int == 200:
            user = json.loads(r.body_string())
            if user["status"]: write_file_atomic("%s.prices.json" % coin_id, json.dumps(user))

        address = coin["address"]
        r = restkit.request("https://api.nanopool.org/v1/%s/user/%s" % (coin_id, address))
        if r.status_int == 200:
            user = json.loads(r.body_string())
            if user["status"]: write_file_atomic("%s.user.json" % coin_id, json.dumps(user))
        r = restkit.request("https://api.nanopool.org/v1/%s/usersettings/%s" % (coin_id, address))
        if r.status_int == 200:
            user = json.loads(r.body_string())
            if user["status"]: write_file_atomic("%s.settings.json" % coin_id, json.dumps(user))
