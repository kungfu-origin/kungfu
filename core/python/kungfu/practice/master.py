import sys
import traceback
import json
import time
import psutil
import functools
from pykungfu import longfist
from pykungfu import yijinjing as yjj
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


class Master(yjj.master):
    def __init__(self, ctx):
        yjj.master.__init__(self, yjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = create_logger("master", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        self.ctx.calendar = Calendar(ctx)
        self.ctx.trading_day = ctx.calendar.trading_day

        self.ctx.master = self

    def is_live_watcher(self, pid):
        info = self.ctx.apprentices[pid]
        location = info['location']
        return info['process'].is_running() and location.category == yjj.category.SYSTEM and location.group == 'node'

    def on_exit(self):
        self.ctx.logger.info('master checking on exit')

        for pid in self.ctx.apprentices:
            apprentice = self.ctx.apprentices[pid]['process']
            if apprentice.is_running():
                self.ctx.logger.info('terminating apprentice %s pid %d', self.ctx.apprentices[pid]['location'].uname, pid)
                self.deregister_app(yjj.now_in_nano(), self.ctx.apprentices[pid]['location'].uid)
                apprentice.terminate()

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = list(
                map(lambda pid: [self.ctx.apprentices[pid]['location'].uname] if self.is_live_watcher(pid) else [],
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

    def on_register(self, event, app_location):
        data = json.loads(event.data_as_string)
        pid = data['pid']
        self.ctx.logger.info('app [%d] %s checking in', pid, app_location.uname)
        if pid not in self.ctx.apprentices:
            try:
                self.ctx.apprentices[pid] = {
                    'process': psutil.Process(pid),
                    'location': app_location
                }
            except (psutil.AccessDenied, psutil.NoSuchProcess):
                exc_type, exc_obj, exc_tb = sys.exc_info()
                err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
                self.ctx.logger.error("app [%d] %s checkin failed: %s", pid, app_location.uname, err_msg)
                self.deregister_app(event.gen_time, app_location.uid)

    def on_interval_check(self, nanotime):
        try:
            run_tasks(self.ctx)
        except Exception:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
            self.ctx.logger.error('task error [%s] %s', exc_type, err_msg)

    def acquire_trading_day(self):
        return self.ctx.calendar.trading_day_ns


@task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            ctx.logger.warn('cleaning up stale app %s with pid %d', ctx.apprentices[pid]['location'].uname, pid)
            ctx.master.deregister_app(yjj.now_in_nano(), ctx.apprentices[pid]['location'].uid)
            del ctx.apprentices[pid]


@task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        ctx.master.publish_trading_day()
