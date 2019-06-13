import psutil
import signal
import pyyjj
import kungfu.yijinjing.io as kfio
from . import os_signal


class Apprentice(pyyjj.apprentice):
    def __init__(self, ctx):
        pyyjj.apprentice.__init__(self, name=ctx.name, low_latency=ctx.low_latency)
        self._ctx = ctx
        self._process = psutil.Process()

        os_signal.handle_os_signals(self.exit_gracefully)

    def go(self):
        kfio.checkin(self._ctx, self._io_device)
        pyyjj.apprentice.go(self)
        # kfio.checkout(self._ctx, self.logger, self._io_device)

    def exit_gracefully(self, signum, frame):
        self.stop()
        if signum == signal.SIGTERM:
            self.logger.info('%s terminated', self._ctx.name)


class EventHandler(pyyjj.event_handler):
    def __init__(self, ctx, handler):
        pyyjj.event_handler.__init__(self)
        self._ctx = ctx
        self._handler = handler

    def configure_event_source(self, event_source):
        self._handler.configure_event_source(event_source)

    def get_name(self):
        return self._handler.get_name()

    def handle(self, event):
        self._handler.handle(event)

    def finish(self):
        self._handler.finish()
