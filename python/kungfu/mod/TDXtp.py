
import xtp
from kungfu.mod.Task import get_task_config
import os
from kungfu.Constants import *

class TDXtp:
    def __init__(self, name):
        self.name = name
        self.xtp = None
        self.config = get_task_config(self.name)
        self.client_id = self.config.get("client_id", 1)
        self.log_level = self.config.get("log_level", SPDLOG_LEVEL_INFO)
        self.software_key = self.config["software_key"]
        self.user_id = self.config["user_id"]
        self.password = self.config["password"]
        self.ip = self.config["td_ip"]
        self.port = self.config["td_port"]
        self.save_path_file = "{}/runtime".format(os.environ['KF_HOME'])

    def run(self):
        self.xtp = xtp.TdGateway(self.client_id, self.software_key, self.save_path_file, self.ip, self.port, self.user_id, self.password, self.log_level)
        self.xtp.init()
        self.xtp.start()

    def stop(self):
        self.xtp.stop()
