import psutil
import signal
import pyyjj
import kungfu.yijinjing.io as kfio
from . import os_signal


class Apprentice(pyyjj.apprentice):
    def __init__(self, args, logger):
        pyyjj.apprentice.__init__(self, name=args.name, low_latency=args.low_latency)
        self._args = args
        self._logger = logger
        self._process = psutil.Process()

        os_signal.handle_os_signals(self.exit_gracefully)
        kfio.checkin(args, logger, self._io_device)

    def exit_gracefully(self, signum, frame):
        self.stop()
        if signum != signal.SIGTERM:
            self._logger.info('%s checking out', self._args.name)
            kfio.checkout(self._args, self._logger, self._io_device)
        else:
            self._logger.info('%s terminated', self._args.name)


class EventHandler(pyyjj.event_handler):
    def __init__(self, logger, handler):
        pyyjj.event_handler.__init__(self)
        self._logger = logger
        self._handler = handler

    def configure_event_source(self, event_source):
        self._handler.configure_event_source(event_source)

    def get_name(self):
        return self._handler.get_name()

    def handle(self, event):
        self._handler.handle(event)

    def finish(self):
        self._handler.finish()
