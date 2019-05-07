import time
import xtp
import threading
from kungfu.mod.Task import get_task_config
import os
from kungfu.Constants import *

class MDXtp:
    def __init__(self):
        self.name = "md_xtp"
        self.xtp = None

    def run(self):
        self.config = get_task_config(self.name)
        self.client_id = self.config.get("client_id", 1)
        self.user_id = self.config["user_id"]
        self.password = self.config["password"]
        self.ip = self.config["md_ip"]
        self.port = self.config["md_port"]
        self.log_level = self.config.get("log_level", SPDLOG_LEVEL_INFO)
        self.save_path_file = "{}/runtime".format(os.environ['KF_HOME'])

        self.xtp = xtp.MdGateway(self.client_id, self.save_path_file, self.ip, self.port, self.user_id, self.password, self.log_level)
        self.xtp.init()
        self.xtp.start()

    def stop(self):
        self.xtp.stop()
