import time
import os
import select
import json
import cffi
import nnpy
from nnpy import nanomsg

import pyyjj
import paged

from kungfu import Constants

class PageManager:
    def __init__(self, name, timeout):
        self.name = name
        self.timeout = timeout

        self.request_handlers = {
            11: self.register_journal,
            12: self.register_client,
            13: self.register_client,
            19: self.exit_client
        }

        base_dir = os.environ["KF_HOME"]
        socket_folder = os.path.join(base_dir, 'socket')
        if not os.path.exists(socket_folder):
            os.path.mkdirs(socket_folder)
        socket_path = os.path.join(socket_folder, 'paged.sock')
        socket_addr = 'ipc://' + socket_path

        self.paged_socket = nnpy.Socket(nnpy.AF_SP, nnpy.REP)
        self.paged_socket.bind(socket_addr)
        self.paged_fd = self.paged_socket.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)

        self.page_engine = paged.PageEngine(base_dir)

    def register_journal(self, request):
        return self.page_engine.reg_journal(request['name'])

    def register_client(self, request):
        comm_file = ''
        file_size = 0
        hash_code = 0
        is_writer = request['type'] == 13
        return self.page_engine.reg_client(comm_file, file_size, hash_code, request['name'], request['pid'], is_writer)

    def exit_client(self, request):
        return self.page_engine.exit_client(request['name'], request['hash_code'], True)
    
    def process_socket_message(self):
        readable, writable, exceptional = select.select([self.paged_fd], [], [self.paged_fd], self.timeout)
        if readable:
            request_data = self.paged_socket.recv()
            request_json = json.loads(request_data.decode('utf-8'))
            response_json = self.request_handlers[request_json['type']](request_json)
            send_bytes = self.paged_socket.send(response_json)

    def start(self):
        self.running = True
        self.run()

    def run(self):
        self.page_engine.start()
        while self.running:
            self.page_engine.process_one_message()
            self.process_socket_message()
        self.page_engine.stop()

    def stop(self):
        self.running = False
