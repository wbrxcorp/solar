import MySQLdb

class Connection:
    def __init__(self, dbname = "solar", autocommit = True):
        self.dbname = dbname
        self.autocommit = autocommit

    def __enter__(self):
        self.conn = MySQLdb.connect(db=self.dbname)
        self.conn.autocommit(self.autocommit)
        self.cursor = self.conn.cursor()
        return self.cursor

    def __exit__(self, exc_type, exc_value, traceback):
        self.cursor.close()
        self.conn.close()
