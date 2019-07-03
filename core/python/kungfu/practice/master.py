import sys
import traceback
import json
import time
import functools
import pyyjj
import pywingchun
import kungfu.service.kfs as kfs
import kungfu.yijinjing.journal as kfj
from kungfu import nanomsg
from kungfu.log import create_logger
from . import os_signal
from kungfu.wingchun.calendar import Calendar

SECOND_IN_NANO = int(1e9)

def get_socket_fd(socket):
    return socket.getsockopt(level=nanomsg.SOL_SOCKET, option=nanomsg.RCVFD)


class Master(pyyjj.master):
    def __init__(self, ctx):
        pyyjj.master.__init__(self, pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        # calendar_db_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator)
        # ctx.calendar = pywingchun.Calendar(ctx.locator.layout_file(calendar_db_location, pyyjj.layout.SQLITE, 'holidays'))
        # ctx.current_trading_day = ctx.calendar.current_trading_day()
        ctx.calendar = Calendar(ctx)
        ctx.current_trading_day = ctx.calendar.current_trading_day()

        os_signal.handle_os_signals(self.exit_gracefully)

    def on_notice(self, event):
        try:
            kfs.handle(event.msg_type, self.ctx, json.loads(event.to_string()))
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('Invalid passive notice %s, [%s] %s', event.to_string(), exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def on_interval_check(self, nanotime):
        kfs.run_tasks(self.ctx)

    def exit_gracefully(self, signum, frame):
        self.ctx.logger.info('kungfu master stopping')

        for pid in self.ctx.apprentices:
            apprentice = self.ctx.apprentices[pid]['process']
            if apprentice.is_running():
                self.ctx.logger.info('terminating apprentice %s pid %d', self.ctx.apprentices[pid]['location'].uname, pid)
                apprentice.terminate()

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = list(
                map(lambda pid: [self.ctx.apprentices[pid]['location'].uname] if self.ctx.apprentices[pid]['process'].is_running() else [],
                    self.ctx.apprentices))
            remaining = functools.reduce(lambda x, y: x + y, remaining) if remaining else []
            if remaining:
                self.ctx.logger.info('terminating apprentices, remaining %s, count down %ds', remaining, time_to_wait - count)
                time.sleep(1)
                count = count + 1
            else:
                break

        for pid in self.ctx.apprentices:
            apprentice = self.ctx.apprentices[pid]['process']
            if apprentice.is_running():
                self.ctx.logger.warn('killing apprentice %s pid %d', self.ctx.apprentices[pid]['location'].uname, pid)
                apprentice.kill()

        self.ctx.logger.info('kungfu master stopped')
