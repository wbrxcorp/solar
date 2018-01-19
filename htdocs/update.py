#!/usr/bin/python
import os,uuid,json,time
import restkit

def write_file_atomic(filename, content):
    tmp_filename = os.path.join(os.path.dirname(os.path.abspath(filename)), str(uuid.uuid4()) + ".tmp")
    with open(tmp_filename, "w") as f:
        f.write(content)
    os.rename(tmp_filename, filename)

def should_download_new_file(filename, ttl_minutes = 60, self_filename = __file__):
    if not os.path.exists(filename): return True
    mtime = os.path.getmtime(filename)
    if os.path.getmtime(self_filename) > mtime: return True
    return  time.time() - mtime / 60 >= ttl_minutes

def get_daily_earnings(coin_id, hashrate):
    prices = json.load(open("%s.prices.json" % coin_id))
    return (prices["data"]["daily_dollars_per_hashrate"] * hashrate, prices["data"]["daily_coins_per_hashrate"] * hashrate)

if __name__ == '__main__':
    filename = "usd.json"
    if should_download_new_file(filename):
        r = restkit.request("https://api.fixer.io/latest?base=USD")
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
                    r2 = restkit.request("https://%s.nanopool.org/api/v1/approximated_earnings/1.0" % coin_id)
                    if r2.status_int == 200:
                        earnings = json.loads(r2.body_string())
                    if earnings["status"]:
                        prices["data"]["daily_dollars_per_hashrate"] = earnings["data"]["day"]["dollars"]
                        prices["data"]["daily_coins_per_hashrate"] = earnings["data"]["day"]["coins"]
                        write_file_atomic(filename, json.dumps(prices))

        address = coin["address"]

        filename = "%s.user.json" % coin_id
        if should_download_new_file(filename):
            r = restkit.request("https://api.nanopool.org/v1/%s/user/%s" % (coin_id, address))
            if r.status_int == 200:
                user = json.loads(r.body_string())
                if user["status"]:
                    h6 = float(user["data"]["avgHashrate"]["h6"])
                    user["data"]["daily_dollars"], user["data"]["daily_coins"] = get_daily_earnings(coin_id, h6)
                    write_file_atomic(filename, json.dumps(user))

        if os.path.isfile(filename):
            user = json.load(open(filename))
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
