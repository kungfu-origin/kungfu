
import time
import sandbox
import threading
from kungfu.mod.Task import get_task_config

class MDSandbox:
    def __init__(self):
        self.name = "md_sandbox"
        self.sandbox = None

    def run(self):
        print(self.name)
        config = get_task_config(self.name)
        account_id = config["account_id"]
        db_file = config["db_file"]
        is_loop = config["is_loop"]
        self.sandbox = sandbox.MdService(account_id, db_file, is_loop)
        thread = threading.Thread(target=self.sandbox.run)
        while True:
            time.sleep(1)

    def stop(self):
        if self.sandbox is not None:
            self.sandbox.stop()
