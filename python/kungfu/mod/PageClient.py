import time
import select
import cffi
import nnpy
from nnpy import ffi, nanomsg

from kungfu import Constants

class PageClient:
    def __init__(self, name, timeout):
        self.name = name
        self.timeout = timeout

        self.subscribers = {}
        self.sub_inputs = []

        sub1 = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
        sub1.connect(Constants.pub_addr)
        sub1.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, '')
        self.sub1_fd = sub1.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)
        self.subscribers[self.sub1_fd] = sub1
        self.sub_inputs.append(self.sub1_fd)

        self.client = nnpy.Socket(nnpy.AF_SP, nnpy.REQ)
        self.client.connect(Constants.reg_addr)

    def run(self):
        while True:
            readable, writable, exceptional = select.select(self.sub_inputs, [], self.sub_inputs, self.timeout)
            for fd in readable:
                print(self.subscribers[fd].recv())
            msg = 'test client ' + str(self.name)
            self.client.send(msg)
            print(msg)
            time.sleep(1)
