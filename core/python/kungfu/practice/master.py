import sys
import traceback
import json
import time
import functools
import pyyjj
import pywingchun

from . import os_signal

from kungfu import nanomsg
from kungfu.log import create_logger

import kungfu.service.kfs as kfs
from kungfu.service.kfs import system
from kungfu.service.kfs import calendar

import kungfu.yijinjing.journal as kfj
import kungfu.yijinjing.time as kft
from kungfu.log import create_logger

from kungfu.wingchun.calendar import Calendar
import kungfu.yijinjing.msg as yjj_msg

SECOND_IN_NANO = int(1e9)


class Master(pyyjj.master):
    def __init__(self, ctx):
        pyyjj.master.__init__(self, pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = create_logger("watcher", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        ctx.calendar = Calendar(ctx)
        ctx.trading_day = ctx.calendar.trading_day
        self.publish_time(yjj_msg.TradingDay, ctx.calendar.trading_day_ns)

        ctx.master = self

    def on_notice(self, event):
        try:
            kfs.handle(event.msg_type, self.ctx, json.loads(event.to_string()))
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('Invalid passive notice %s, [%s] %s', event.to_string(), exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def on_interval_check(self, nanotime):
        kfs.run_tasks(self.ctx)

    def on_register(self, event):
        self.send_time(event.source, yjj_msg.TradingDay, self.ctx.calendar.trading_day_ns)

    def on_exit(self):
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
