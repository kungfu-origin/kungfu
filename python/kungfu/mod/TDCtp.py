import time
import ctp
import threading
from kungfu.mod.Task import get_task_config
from kungfu.Constants import *

class TDCtp:
    def __init__(self, name):
        self.ctp = None
        self.name = name

    def run(self):
        self.config = get_task_config(self.name)
        print("run {} with config {}".format(self.name, self.config))
        self.account_id = self.config["account_id"]
        self.broker_id = self.config["broker_id"]
        self.password = self.config["password"]
        self.front_uri = self.config["td_uri"]
        self.log_level = self.config.get("log_level", SPDLOG_LEVEL_TRACE)

        self.ctp = ctp.TdGateway(self.front_uri, self.broker_id, self.account_id, self.password, self.log_level)
        self.ctp.init()
        self.ctp.start()

    def stop(self):
        self.ctp.stop()
    