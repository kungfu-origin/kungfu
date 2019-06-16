import sys
import traceback
import select
import json
import time
import functools

import pyyjj
import pywingchun
import kungfu.service.kfs as kfs
import kungfu.yijinjing.journal as kfj
from kungfu import nanomsg
from . import os_signal

SECOND_IN_NANO = int(1e9)


def get_socket_fd(socket):
    return socket.getsockopt(level=nanomsg.SOL_SOCKET, option=nanomsg.RCVFD)


class Master(pyyjj.master):
    def __init__(self, ctx):
        pyyjj.master.__init__(self, pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.logger = ctx.logger

        self.apprentices = {}
        self._check_interval = 5 * SECOND_IN_NANO

        self._calendar = pywingchun.CalendarService()
        self._current_day = self._calendar.current_day()
        self._running = False
        self._last_check = 0

        os_signal.handle_os_signals(self.exit_gracefully)

    def process_passive_notice(self):
        readable, writable, exceptional = select.select([self._fd_pull], [], [], self._pull_timeout)
        if readable:
            raw = self._socket_pull.recv()
            try:
                event_msg = json.dumps({
                    'gen_time': pyyjj.now_in_nano(),
                    'trigger_time': 0,
                    'msg_type': 999,
                    'source': 0,
                    'data': json.loads(raw)
                }) if len(raw) > 2 else raw
                rc = self._socket_publish.send(event_msg)
                self.logger.debug('Published passive notice: %d, %s', rc, event_msg)
            except Exception as err:
                exc_type, exc_obj, exc_tb = sys.exc_info()
                self.logger.error('Invalid passive notice %s, [%s] %s', raw, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
                self._socket_publish.send('{}')

    def process_service_message(self):
        readable, writable, exceptional = select.select([self._fd_reply], [], [], 0)
        if readable:
            request_data = self._socket_reply.recv()
            self.logger.debug('received %s', request_data)
            request_json = json.loads(request_data)
            request_path = request_json['request']
            response = kfs.handle(request_path, self, request_json)
            self._socket_reply.send(json.dumps(response))
            self.logger.debug('processed request %s, response %s', request_json, response)

    def run_tasks(self):
        time_passed = pyyjj.now_in_nano() - self._last_check
        if time_passed > self._check_interval:
            kfs.run_tasks(self)
            self._last_check = pyyjj.now_in_nano()

    def go1(self):
        self._running = True
        self._last_check = pyyjj.now_in_nano()
        while self._running:
            try:
                self.page_service.process_memory_message()
                self.process_passive_notice()
                self.process_service_message()
                self.run_tasks()
            except:
                if self._running:
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    self.logger.error('Server suffers: %s %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
        self.logger.info('kungfu master finished')

    def exit_gracefully(self, signum, frame):
        self.logger.info('kungfu master stopping')

        for pid in self.apprentices:
            apprentice = self.apprentices[pid]['process']
            if apprentice.is_running():
                self.logger.info('terminating apprentice %s pid %d', self.apprentices[pid]['name'], pid)
                apprentice.terminate()

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = list(map(lambda pid: [self.apprentices[pid]['name']] if self.apprentices[pid]['process'].is_running() else [], self.apprentices))
            remaining = functools.reduce(lambda x, y: x + y, remaining) if remaining else []
            if remaining:
                self.logger.info('terminating apprentices, remaining %s, count down %ds', remaining, time_to_wait - count)
                time.sleep(1)
                count = count + 1
            else:
                break

        for pid in self.apprentices:
            apprentice = self.apprentices[pid]['process']
            if apprentice.is_running():
                self.logger.warn('killing apprentice %s pid %d', self.apprentices[pid]['name'], pid)
                apprentice.kill()

        self._running = False

        self.logger.info('kungfu master stopped')
