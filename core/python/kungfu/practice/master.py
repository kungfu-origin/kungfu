import sys
import traceback
import json
import time
import psutil
import functools
import pyyjj
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.msg as yjj_msg
import kungfu.yijinjing.journal as kfj
from kungfu.yijinjing.log import create_logger

from kungfu.wingchun.calendar import Calendar
import kungfu.yijinjing.msg as yjj_msg

SECOND_IN_NANO = int(1e9)
TASKS = dict()


def task(func):
    @functools.wraps(func)
    def task_wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    TASKS[func.__name__] = task_wrapper
    return task_wrapper


def run_tasks(*args, **kwargs):
    for task_name in TASKS:
        TASKS[task_name](*args, **kwargs)


class Master(pyyjj.master):
    def __init__(self, ctx):
        pyyjj.master.__init__(self, pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = create_logger("master", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        ctx.calendar = Calendar(ctx)
        ctx.trading_day = ctx.calendar.trading_day
        self.publish_time(yjj_msg.TradingDay, ctx.calendar.trading_day_ns)

        ctx.master = self

    def on_interval_check(self, nanotime):
        try:
            run_tasks(self.ctx)
        except Exception as err:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            self.ctx.logger.error('task error [%s] %s', exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))

    def on_register(self, event, app_location):
        data = json.loads(event.data_as_string)
        pid = data['pid']
        self.ctx.logger.info('app %s %d checking in', app_location.uname, pid)
        if pid not in self.ctx.apprentices:
            try:
                self.ctx.apprentices[pid] = {
                    'process': psutil.Process(pid),
                    'location': app_location
                }
            except (psutil.AccessDenied, psutil.NoSuchProcess):
                self.ctx.logger.warn("app %s %d does not exist", app_location.uname, pid)
        self.send_time(event.source, yjj_msg.TradingDay, self.ctx.calendar.trading_day_ns)

    def on_exit(self):
        pyyjj.master.on_exit(self)
        self.ctx.logger.info('master checking on exit')

        for pid in self.ctx.apprentices:
            apprentice = self.ctx.apprentices[pid]['process']
            if apprentice.is_running():
                self.ctx.logger.info('terminating apprentice %s pid %d', self.ctx.apprentices[pid]['location'].uname, pid)
                self.deregister_app(pyyjj.now_in_nano(), self.ctx.apprentices[pid]['location'].uid)
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

        self.ctx.logger.info('master cleaned up')


@task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            ctx.logger.warn('cleaning up stale app %s with pid %d', ctx.apprentices[pid]['location'].uname, pid)
            ctx.master.deregister_app(pyyjj.now_in_nano(), ctx.apprentices[pid]['location'].uid)
            del ctx.apprentices[pid]


@task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        ctx.master.publish_time(yjj_msg.TradingDay, ctx.calendar.trading_day_ns)
