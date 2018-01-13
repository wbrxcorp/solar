#!/usr/bin/python
import os,uuid,json,time
import restkit

def write_file_atomic(filename, content):
    tmp_filename = os.path.join(os.path.dirname(os.path.abspath(filename)), str(uuid.uuid4()) + ".tmp")
    with open(tmp_filename, "w") as f:
        f.write(content)
    os.rename(tmp_filename, filename)

def should_download_new_file(filename, ttl_minutes = 60):
    if not os.path.exists(filename): return False
    age = time.time() - os.path.getmtime(filename)
    return age / 60 >= ttl_minutes

if __name__ == '__main__':
    filename = "usd.json"
    if should_download_new_file(filename):
        r = restkit.request("https://api.fixer.io/latest?base=USD")
        if r.status_int == 200: write_file_atomic(filename, r.body_string())

    filename = "btc.json"
    if should_download_new_file(filename):
        r = restkit.request("https://api.coindesk.com/v1/bpi/currentprice.json")
        if r.status_int == 200: write_file_atomic(filename, r.body_string())

    coins = json.load(open("coins.json"))

    for coin in coins:
        coin_id = coin["id"]

        filename = "%s.prices.json" % coin_id
        if should_download_new_file(filename):
            r = restkit.request("https://api.nanopool.org/v1/%s/prices" % coin_id)
            if r.status_int == 200:
                user = json.loads(r.body_string())
                if user["status"]: write_file_atomic(filename, json.dumps(user))

        address = coin["address"]

        filename = "%s.user.json" % coin_id
        if should_download_new_file(filename):
            r = restkit.request("https://api.nanopool.org/v1/%s/user/%s" % (coin_id, address))
            if r.status_int == 200:
                user = json.loads(r.body_string())
                if user["status"]: write_file_atomic(filename, json.dumps(user))

        filename = "%s.settings.json" % coin_id
        if should_download_new_file(filename):
            r = restkit.request("https://api.nanopool.org/v1/%s/usersettings/%s" % (coin_id, address))
            if r.status_int == 200:
                user = json.loads(r.body_string())
                if user["status"]: write_file_atomic(filename, json.dumps(user))
