import pyyjj
import signal


def handle_os_signals(handler):
    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)
    signal.signal(signal.SIGHUP, handler)
    signal.signal(signal.SIGBUS, handler)
    signal.signal(signal.SIGSEGV, handler)
    signal.signal(signal.SIGABRT, handler)
