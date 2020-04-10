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
        yjj.master.__init__(self, yjj.location(kfj.MODES['live'], kfj.CATEGORIES['system'], 'master', 'master', ctx.locator), ctx.low_latency)
        self.ctx = ctx
        self.ctx.master = self
        self.ctx.logger = log.create_logger("master", ctx.log_level, self.io_device.home)
        self.ctx.apprentices = {}

        self.ctx.calendar = Calendar(ctx)
        self.ctx.trading_day = ctx.calendar.trading_day

        self.ctx.master = self

        self.profile = yjj.profile(ctx.locator)
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

    def is_live_node_process(self, pid):
        info = self.ctx.apprentices[pid]
        register = info['register']
        return info['process'].is_running() and register.category == yjj.category.SYSTEM and register.group == 'node'

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
                    'uname': uname,
                    'register': register_data
                }
            except (psutil.AccessDenied, psutil.NoSuchProcess):
                exc_type, exc_obj, exc_tb = sys.exc_info()
                err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
                self.ctx.logger.error("app [%d] %s checkin failed: %s", pid, uname, err_msg)
                self.deregister_app(event.gen_time, register_data.location_uid)

    def on_interval_check(self, nanotime):
        try:
            run_tasks(self.ctx)
        except Exception:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            err_msg = traceback.format_exception(exc_type, exc_obj, exc_tb)
            self.ctx.logger.error('task error [%s] %s', exc_type, err_msg)

    def on_exit(self):
        for pid in self.ctx.apprentices:
            apprentice = self.ctx.apprentices[pid]['process']
            if apprentice.is_running():
                self.ctx.logger.info('terminating apprentice %s pid %d', self.ctx.apprentices[pid]['uname'], pid)
                self.deregister_app(yjj.now_in_nano(), self.ctx.apprentices[pid]['register'].uid)
                apprentice.terminate()

        count = 0
        time_to_wait = 10
        while count < time_to_wait:
            remaining = list(
                map(lambda pid: [self.ctx.apprentices[pid]['uname']] if self.is_live_node_process(pid) else [],
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
                self.ctx.logger.warn('killing apprentice %s pid %d', self.ctx.apprentices[pid]['uname'], pid)
                apprentice.kill()

        self.ctx.logger.info('master cleaned up')


@task
def health_check(ctx):
    for pid in list(ctx.apprentices.keys()):
        if not ctx.apprentices[pid]['process'].is_running():
            ctx.logger.warn('cleaning up stale app %s with pid %d', ctx.apprentices[pid]['uname'], pid)
            ctx.master.deregister_app(yjj.now_in_nano(), ctx.apprentices[pid]['register'].uid)
            del ctx.apprentices[pid]


@task
def switch_trading_day(ctx):
    trading_day = ctx.calendar.trading_day
    if ctx.trading_day < trading_day:
        ctx.trading_day = trading_day
        ctx.master.publish_trading_day()
