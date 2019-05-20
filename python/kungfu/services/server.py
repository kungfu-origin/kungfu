import os
import select
import json
import nnpy, pyyjj

from kungfu.services.handlers.paged import *
from kungfu.services.handlers.calendar import *
from kungfu.services.handlers.system import *
from kungfu.services.handlers import kfs_handle, kfs_run_tasks

SECOND_IN_NANO = 1000000000

class Server:
    def __init__(self, logger):
        self.process = psutil.Process()
        self.logger = logger

        base_dir = os.getenv('KF_HOME')
        self.page_service = pyyjj.PageService(base_dir)

        socket_folder = os.path.join(base_dir, 'socket')
        if not os.path.exists(socket_folder):
            os.makedirs(socket_folder)

        self.emitter_socket, self.emitter_fd = self.create_socket(socket_folder, 'emitter', nnpy.PULL)
        self.notice_socket, no_fd = self.create_socket(socket_folder, 'notice', nnpy.PUB)
        self.service_socket, self.service_fd = self.create_socket(socket_folder, 'service', nnpy.REP)

        self.client_info = {}
        self.client_processes = {}
        self.check_interval = 5 * SECOND_IN_NANO
    
    def create_socket(self, socket_folder, name, nn_mode):
        socket_path = os.path.join(socket_folder, name + '.sock')
        socket_addr = 'ipc://' + socket_path
        socket = nnpy.Socket(nnpy.AF_SP, nn_mode)
        socket.bind(socket_addr)
        fd = None
        if nn_mode != nnpy.PUB:
            fd = socket.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)
        self.logger.info('%s ready: %s', name, socket_path)
        return socket, fd

    def process_service_message(self):
        readable, writable, exceptional = select.select([self.service_fd], [], [], 0)
        if readable:
            request_data = self.service_socket.recv()
            request_json = json.loads(request_data.decode('utf-8'))
            request_path = request_json['request']
            response = kfs_handle(request_path, self, request_json)
            send_bytes = self.service_socket.send(json.dumps(response))
    
    def process_passive_notice(self):
        readable, writable, exceptional = select.select([self.emitter_fd], [], [], 1)
        if readable:
            event_msg_data = self.emitter_socket.recv()
            try:
                event_msg_str = event_msg_data.decode('utf-8')
                event_msg = json.loads(event_msg_str)
                rc = self.notice_socket.send(event_msg_str)
                self.logger.debug('Published passive notice: %d, %s', rc, event_msg)
            except:
                self.logger.error('Invalid passive notice: %s', event_msg_data)
                self.notice_socket.send("{}")

    def run_tasks(self):
        time_passed = pyyjj.nano_time() - self.last_check
        if time_passed > self.check_interval:
            self.logger.debug('Run system tasks')
            kfs_run_tasks(self)
            self.last_check = pyyjj.nano_time()

    def start(self):
        self.running = True
        self.last_check = pyyjj.nano_time()
        self.run()

    def run(self):
        self.page_service.start()
        while self.running:
            self.process_passive_notice()
            self.page_service.process_memory_message()
            self.process_service_message()
            self.run_tasks()
        self.service_socket.close()
        self.emitter_socket.close()
        self.notice_socket.close()
        self.page_service.stop()

    def stop(self):
        self.running = False
