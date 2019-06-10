import sys, traceback
import select
import json
import psutil
import time
import functools

import pyyjj
import pywingchun
import kungfu.service.kfs as kfs
from kungfu import nanomsg
from . import os_signal

SECOND_IN_NANO = int(1e9)


def get_socket_fd(socket):
    return socket.getsockopt(level=nanomsg.SOL_SOCKET, option=nanomsg.RCVFD)


class Master:
    def __init__(self, args, logger):
        pyyjj.setup_log(args.name)
        self._logger = logger

        self._io_device = pyyjj.create_io_device(args.name, args.low_latency)
        self._page_service = pyyjj.page_service(self._io_device)

        self._pull_timeout = 0 if args.low_latency else 1
        self._socket_pull = self._io_device.bind_socket(pyyjj.mode.LIVE, category=pyyjj.category.SYSTEM, group=args.name, name=args.name, protocol=pyyjj.protocol.PULL)
        self._socket_reply = self._io_device.bind_socket(pyyjj.mode.LIVE, category=pyyjj.category.SYSTEM, group=args.name, name=args.name, protocol=pyyjj.protocol.REPLY)
        self._socket_publish = self._io_device.bind_socket(pyyjj.mode.LIVE, category=pyyjj.category.SYSTEM, group=args.name, name=args.name, protocol=pyyjj.protocol.PUBLISH)

        self._fd_pull = get_socket_fd(self._socket_pull)
        self._fd_reply = get_socket_fd(self._socket_reply)

        self._apprentices = {}
        self._check_interval = 5 * SECOND_IN_NANO

        self._calendar = pywingchun.CalendarService()
        self._current_day = self._calendar.current_day()
        self._running = False
        self._last_check = 0

        os_signal.handle_os_signals(self.exit_gracefully)

    def process_passive_notice(self):
        readable, writable, exceptional = select.select([self._fd_pull], [], [], self._pull_timeout)
        if readable:
            event_msg_data = self._socket_pull.recv()
            try:
                event_msg_str = event_msg_data
                event_msg = json.loads(event_msg_str)
                rc = self._socket_publish.send(event_msg_str)
                self._logger.debug('Published passive notice: %d, %s', rc, event_msg)
            except Exception as err:
                exc_type, exc_obj, exc_tb = sys.exc_info()
                self._logger.error('Invalid passive notice %s, [%s] %s', event_msg_data, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
                self._socket_publish.send('{}')

    def process_service_message(self):
        readable, writable, exceptional = select.select([self._fd_reply], [], [], 0)
        if readable:
            request_data = self._socket_reply.recv()
            self._logger.debug('received %s', request_data)
            request_json = json.loads(request_data)
            request_path = request_json['request']
            response = kfs.handle(request_path, self, request_json)
            self._socket_reply.send(json.dumps(response))
            self._logger.debug('processed request %s, response %s', request_json, response)

    def run_tasks(self):
        time_passed = pyyjj.now_in_nano() - self._last_check
        if time_passed > self._check_interval:
            kfs.run_tasks(self)
            self._last_check = pyyjj.now_in_nano()

    def go(self):
        self._running = True
        self._last_check = pyyjj.now_in_nano()
        while self._running:
            try:
                self._page_service.process_memory_message()
                self.process_passive_notice()
                self.process_service_message()
                self.run_tasks()
            except:
                if self._running:
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    self._logger.error('Server suffers: %s %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        self._logger.info('kungfu master finished')

    def exit_gracefully(self, signum, frame):
        self._logger.info('kungfu master stopping')

        for pid in self._apprentices:
            apprentice = self._apprentices[pid]['process']
            if apprentice.is_running():
                self._logger.info('terminating apprentice %s pid %d', self._apprentices[pid]['name'], pid)
                apprentice.terminate()

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = list(map(lambda pid: [self._apprentices[pid]['name']] if self._apprentices[pid]['process'].is_running() else [], self._apprentices))
            remaining = functools.reduce(lambda x, y: x + y, remaining) if remaining else []
            if remaining:
                self._logger.info('terminating apprentices, remaining %s, count down %ds', remaining, time_to_wait - count)
                time.sleep(1)
                count = count + 1
            else:
                break

        for pid in self._apprentices:
            apprentice = self._apprentices[pid]['process']
            if apprentice.is_running():
                self._logger.warn('killing apprentice %s pid %d', self._apprentices[pid]['name'], pid)
                apprentice.kill()

        self._running = False
        self._socket_reply.close()
        self._socket_pull.close()
        self._socket_publish.close()

        self._logger.info('kungfu master stopped')
