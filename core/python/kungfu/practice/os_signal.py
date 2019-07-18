import pyyjj
import signal
import platform


def handle_os_signals(handler):
    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)
    signal.signal(signal.SIGSEGV, handler)
    if platform.system() != 'Windows':
        signal.signal(signal.SIGHUP, handler)
        signal.signal(signal.SIGABRT, handler)
        signal.signal(signal.SIGBUS, handler)
