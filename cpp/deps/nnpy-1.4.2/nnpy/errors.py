from _nnpy import ffi, lib as nanomsg

class NNError(Exception):
    def __init__(self, error_no, *args, **kwargs):
        super(NNError, self).__init__(*args, **kwargs)
        self.error_no = error_no

def convert(rc, value=None):
    if rc < 0:
        error_no = nanomsg.nn_errno()
        chars = nanomsg.nn_strerror(error_no)
        msg = ffi.string(chars).decode()
        raise NNError(error_no, msg)
    if callable(value):
        return value()
    return value

