import time
import select
import cffi
import nnpy
from nnpy import ffi, nanomsg

import pyyjj
import paged

from kungfu import Constants

class PageManager:
    def __init__(self, name, timeout, base_dir):
        self.name = name
        self.timeout = timeout

        self.pub = nnpy.Socket(nnpy.AF_SP, nnpy.PUB)
        self.pub.bind(Constants.pub_addr)

        self.clients = {}
        self.client_inputs = []

        self.reg_sock = nnpy.Socket(nnpy.AF_SP, nnpy.REP)
        self.reg_sock.bind(Constants.reg_addr)
        reg_fd = self.reg_sock.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)
        self.clients[reg_fd] = self.reg_sock
        self.client_inputs.append(reg_fd)

        self.page_engine = paged.PageEngine(base_dir)
        self.running = False
        self.finished = False

    def start(self):
        self.running = True
        self.run()

    def run(self):
        self.pub.send('now: ' + str(pyyjj.nano()))
        self.page_engine.start()
        while self.running:
            self.pub.send('now: ' + str(pyyjj.nano()))
            readable, writable, exceptional = select.select(self.client_inputs, [], self.client_inputs, self.timeout)
            for fd in readable:
                print(str(fd) + ': ' + str(self.clients[fd].recv()))
            self.pub.send(str(self.page_engine.status()))
        self.page_engine.stop()
        self.finished = True

    def stop(self):
        self.running = False
        while not self.finished:
            time.sleep(self.timeout)
