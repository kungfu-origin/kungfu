from .constants import *
from _nnpy import ffi, lib as nanomsg
from . import errors
import os

from .errors import NNError
from .socket import Socket

class PollSet(object):

    def __init__(self, *args):
        self.data = [(sock.sock, events, 0) for (sock, events) in args]
        self.fd_set = ffi.new('struct nn_pollfd[]', self.data)

    def poll(self, timeout=0):
        rc = nanomsg.nn_poll(self.fd_set, len(self.data), timeout)
        return errors.convert(rc, lambda: [fd.revents for fd in self.fd_set])
