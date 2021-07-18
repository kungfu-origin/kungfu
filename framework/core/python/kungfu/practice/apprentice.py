import psutil
import signal
from pykungfu import yijinjing as yjj
import kungfu.yijinjing.io as kfio
import kungfu.yijinjing.journal as kfj
from . import os_signal


class Apprentice(yjj.apprentice):
    def __init__(self, ctx):
        yjj.apprentice.__init__(self, yjj.location(kfj.MODES[ctx.mode], kfj.CATEGORIES[ctx.category], ctx.group, ctx.name, ctx.runtime_locator),
                                  low_latency=ctx.low_latency)
        self.ctx = ctx
        self._process = psutil.Process()

        os_signal.handle_os_signals(self.exit_gracefully)

    def go(self):
        kfio.checkin(self.ctx, self.io_device)
        yjj.apprentice.go(self)
        # kfio.checkout(self._ctx, self.logger, self._io_device)

    def exit_gracefully(self, signum, frame):
        self.stop()
        if signum == signal.SIGTERM:
            self.logger.info('%s terminated', self.ctx.name)
