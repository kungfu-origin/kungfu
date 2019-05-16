import time
import os
import select
import json
import psutil
import nnpy

import pyyjj
from kungfu import Constants

class PageServer:
    def __init__(self):
        self.request_handlers = {
            11: self.register_journal,
            12: self.register_client,
            13: self.register_client,
            19: self.exit_client
        }

        base_dir = os.getenv('KF_HOME')
        socket_folder = os.path.join(base_dir, 'socket')
        if not os.path.exists(socket_folder):
            os.path.mkdirs(socket_folder)
        socket_path = os.path.join(socket_folder, 'paged.sock')
        socket_addr = 'ipc://' + socket_path

        self.paged_socket = nnpy.Socket(nnpy.AF_SP, nnpy.REP)
        self.paged_socket.bind(socket_addr)
        self.paged_fd = self.paged_socket.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)

        self.clients = {}
        self.processes = {}
        self.page_service = pyyjj.PageService(base_dir)

    def register_journal(self, request):
        name = request['name']
        result = {
            'type': 11,
            'success': False,
            'error_msg': '',
            'memory_msg_idx': -1
        }
        if not name in self.clients:
            result['error_msg'] = 'client ' + name + ' does not exist'
        else:
            client = self.clients[name]
            idx = self.page_service.register_journal(name)
            if idx >= 1000:
                result['error_msg'] = 'idx exceeds limit'
            else:
                client['journals'].append(idx)
                result['success'] = True
                result['memory_msg_idx'] = idx
        return json.dumps(result)

    def register_client(self, request):
        name = request['name']
        pid = request['pid']
        is_writer = request['type'] == 13
        result = {
            'type': 13 if is_writer else 12,
            'success': False,
            'error_msg': '',
            'memory_msg_file': self.page_service.memory_msg_file(),
            'file_size': self.page_service.memory_msg_file_size(),
            'hash_code': -1
        }
        if name in self.clients:
            result['error_msg'] = 'client ' + name + ' already exists'
        else:
            hash_code = self.page_service.register_client(name, pid, is_writer)
            if not pid in self.processes:
                self.processes[pid] = {}
                self.processes[pid]['process'] = psutil.Process(pid)
                self.processes[pid]['clients'] = []
            self.processes[pid]['clients'].append(name)
            self.clients[name] = {
                'journals': [],
                'reg_nano': pyyjj.nano_time(),
                'is_writing': is_writer,
                'is_strategy': False,
                'rid_start': -1,
                'rid_end': -1,
                'pid': pid,
                'hash_code': hash_code
            }
            result['success'] = True
            result['hash_code'] = hash_code
        return json.dumps(result)

    def exit_client(self, request):
        name = request['name']
        hash_code = request['hash_code']
        result = {
            'type': 19,
            'success': False,
            'error_msg': ''
        }
        self.release_client(name, hash_code, True, result)
        return json.dumps(result)

    def release_client(self, name, hash_code, validation, result):
        if not name in self.clients:
            result['error_msg'] = 'client ' + name + ' does not exist'
            return
        client = self.clients[name]
        if validation and hash_code != client['hash_code']:
            result['error_msg'] = 'hash code validation failed ' + str(hash_code) + '!=' + str(client['hash_code'])
        else:
            for idx in client['journals']:
                self.page_service.release_page(idx)
            del self.clients[name]
            result['success'] = True
    
    def process_socket_message(self):
        readable, writable, exceptional = select.select([self.paged_fd], [], [self.paged_fd], 0)
        if readable:
            request_data = self.paged_socket.recv()
            request_json = json.loads(request_data.decode('utf-8'))
            response_json = self.request_handlers[request_json['type']](request_json)
            send_bytes = self.paged_socket.send(response_json)

    def health_check(self):
        stale_pids = []
        for pid in self.processes:
            if not self.processes[pid]['process'].is_running():
                for name in self.processes[pid]['clients']:
                    self.release_client(name, 0, False, {})
                stale_pids.append(pid)
        for pid in stale_pids:
            del self.processes[pid]

    def start(self):
        self.running = True
        self.run()

    def run(self):
        self.page_service.start()
        while self.running:
            self.page_service.process_memory_message()
            self.process_socket_message()
            self.health_check()
        self.page_service.stop()

    def stop(self):
        self.running = False
