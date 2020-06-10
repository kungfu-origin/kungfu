import sys
import time
import psutil
import functools
import traceback

from kungfu.yijinjing import journal as kfj
from kungfu.yijinjing import log
from kungfu.wingchun import default_commissions
from kungfu.wingchun.calendar import Calendar
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj

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
        yjj.master.__init__(self, yjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.runtime_locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = log.create_logger("master", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        self.ctx.calendar = Calendar(ctx)
        self.ctx.trading_day = ctx.calendar.trading_day

        self.ctx.master = self

        self.profile = yjj.profile(ctx.runtime_locator)
        self.commissions = {}
        for commission in self.profile.get_all(lf.types.Commission()):
            self.commissions[commission.product_id] = commission
        default_commissions.apply(lambda default: self.set_default_commission(default), 1)

    def set_default_commission(self, default):
        if default.product_id not in self.commissions:
            commission = lf.types.Commission()
            commission.product_id = default.product_id
            commission.exchange_id = default.exchange_id
            commission.instrument_type = lf.enums.InstrumentType(default['instrument_type'])
            commission.mode = lf.enums.CommissionRateMode(default['mode'])
            commission.open_ratio = default.open_ratio
            commission.close_ratio = default.close_ratio
            commission.close_today_ratio = default.close_today_ratio
            commission.min_commission = default.min_commission
            self.profile.set(commission)

    def acquire_trading_day(self):
        return self.ctx.calendar.trading_day_ns

    def on_register(self, event, register_data):
        pid = register_data.pid
        category = yjj.get_category_name(register_data.category)
        mode = yjj.get_mode_name(register_data.mode)
        uname = f'{category}/{register_data.group}/{register_data.name}/{mode}'
        self.ctx.logger.info(f'app {pid} {uname} checking in')
        if pid not in self.ctx.apprentices:
            try:
                self.ctx.apprentices[pid] = {
                    'process': psutil.Process(pid),
                    'pid': pid,
                    'uname': uname,
                    'register': register_data
                }
            except (psutil.AccessDenied, psutil.NoSuchProcess):
                exc_type, exc_obj, exc_tb = sys.exc_info()
                err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
                self.ctx.logger.error(f'app [{pid}] {uname} checkin failed: {err_msg}')
                self.deregister_app(event.gen_time, register_data.location_uid)

    def on_interval_check(self, nanotime):
        try:
            run_tasks(self.ctx)
        except RuntimeError:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
            self.ctx.logger.error('task error [%s] %s', exc_type, err_msg)

    def on_exit(self):
        for app in self.get_live_processes():
            self.ctx.logger.info(f'terminating apprentice {app["uname"]} pid {app["pid"]}')
            self.deregister_app(yjj.now_in_nano(), app['register'].__uid__)
            try:
                app['process'].terminate()
            except psutil.Error:
                self.ctx.logger.error(f'failed to terminate apprentice {app["uname"]} pid {app["pid"]}')

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = self.get_live_processes()
            if remaining:
                names = list(map(lambda app: app['uname'], remaining))
                self.ctx.logger.info(f'terminating apprentices, remaining {names}, count down {time_to_wait - count}s')
                time.sleep(1)
                count = count + 1
            else:
                break

        for app in self.get_live_processes():
            self.ctx.logger.warn(f'killing apprentice {app["uname"]} pid {app["pid"]}')
            try:
                app['process'].kill()
            except psutil.Error:
                self.ctx.logger.error(f'failed to kill apprentice {app["uname"]} pid {app["pid"]}')

        self.ctx.logger.info('master cleaned up')

    def is_live_process(self, pid):
        return pid in self.ctx.apprentices and self.ctx.apprentices[pid]['process'].is_running()

    def is_node_process(self, pid):
        if pid not in self.ctx.apprentices:
            return False
        registry = self.ctx.apprentices[pid]['register']
        return registry.category == yjj.category.SYSTEM and registry.group == 'node'

    def filter_live_process(self, pid):
        return [self.ctx.apprentices[pid]] if self.is_live_process(pid) and not self.is_node_process(pid) else []

    def get_live_processes(self):
        remaining = list(map(self.filter_live_process, self.ctx.apprentices))
        return functools.reduce(lambda x, y: x + y, remaining) if remaining else []


@task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            app = ctx.apprentices[pid]
            ctx.logger.warn(f'cleaning up stale app {app["uname"]} with pid {pid}')
            ctx.master.deregister_app(yjj.now_in_nano(), app['register'].__uid__)
            del ctx.apprentices[pid]


@task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        ctx.master.publish_trading_day()
