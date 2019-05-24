import os, sys, traceback
import select
import json
import psutil
import nnpy, pyyjj
import pywingchun
import kungfu.service.kfs as kfs
from kungfu.command import command, arg

@command(help='kungfu master')
def master(args, logger):
    server = Master(logger)
    server.start()

SECOND_IN_NANO = int(1e9)

class Master:
    def __init__(self, logger):
        self._process = psutil.Process()
        self._logger = logger

        base_dir = os.getenv('KF_HOME')
        self._page_service = pyyjj.PageService(base_dir)

        socket_folder = os.path.join(base_dir, 'socket')
        if not os.path.exists(socket_folder):
            os.makedirs(socket_folder)

        self._emitter_socket, self._emitter_fd = self.create_socket(socket_folder, 'emitter', nnpy.PULL)
        self._notice_socket, no_fd = self.create_socket(socket_folder, 'notice', nnpy.PUB)
        self._service_socket, self._service_fd = self.create_socket(socket_folder, 'service', nnpy.REP)

        self._client_info = {}
        self._client_processes = {}
        self._check_interval = 5 * SECOND_IN_NANO

        self._calendar = pywingchun.CalendarService()
        self._current_day = self._calendar.current_day()
    
    def create_socket(self, socket_folder, name, nn_mode):
        socket_path = os.path.join(socket_folder, name + '.sock')
        socket_addr = 'ipc://' + socket_path
        socket = nnpy.Socket(nnpy.AF_SP, nn_mode)
        socket.bind(socket_addr)
        fd = None
        if nn_mode != nnpy.PUB:
            fd = socket.getsockopt(level=nnpy.SOL_SOCKET, option=nnpy.RCVFD)
        self._logger.info('%s ready: %s', name, socket_path)
        return socket, fd

    def process_service_message(self):
        readable, writable, exceptional = select.select([self._service_fd], [], [], 0)
        if readable:
            request_data = self._service_socket.recv()
            self._logger.debug("received %s", request_data.decode('utf-8'))
            request_json = json.loads(request_data.decode('utf-8'))
            request_path = request_json['request']
            response = kfs.handle(request_path, self, request_json)
            send_bytes = self._service_socket.send(json.dumps(response))
            self._logger.debug("processed request %s, response %s", request_json, response)
    
    def process_passive_notice(self):
        readable, writable, exceptional = select.select([self._emitter_fd], [], [], 1)
        if readable:
            event_msg_data = self._emitter_socket.recv()
            try:
                event_msg_str = event_msg_data.decode('utf-8')
                event_msg = json.loads(event_msg_str)
                rc = self._notice_socket.send(event_msg_str)
                self._logger.debug('Published passive notice: %d, %s', rc, event_msg)
            except Exception as err:
                exc_type, exc_obj, exc_tb = sys.exc_info()
                self._logger.error('Invalid passive notice %s, [%s] %s', event_msg_data, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
                self._notice_socket.send("{}")

    def run_tasks(self):
        time_passed = pyyjj.nano_time() - self._last_check
        if time_passed > self._check_interval:
            self._logger.debug('Run system tasks')
            kfs.run_tasks(self)
            self._last_check = pyyjj.nano_time()

    def start(self):
        self._running = True
        self._last_check = pyyjj.nano_time()
        self.run()

    def run(self):
        self._page_service.start()
        while self._running:
            try:
                self.process_passive_notice()
                self._page_service.process_memory_message()
                self.process_service_message()
                self.run_tasks()
            except Exception as err:
                exc_type, exc_obj, exc_tb = sys.exc_info()
                self._logger.error("Server suffers: %s %s", exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        self._service_socket.close()
        self._emitter_socket.close()
        self._notice_socket.close()
        self._page_service.stop()

    def stop(self):
        self._running = False
