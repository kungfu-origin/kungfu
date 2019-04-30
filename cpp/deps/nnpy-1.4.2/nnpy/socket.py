from . import errors, ffi, nanomsg
import sys

NN_MSG = int(ffi.cast("size_t", -1))

ustr = str if sys.version_info[0] > 2 else unicode

class Socket(object):
    """
    Nanomsg scalability protocols (SP) socket.

    .. seealso:: `nanomsg <http://nanomsg.org/v1.0.0/nanomsg.7.html>`_
    """
    def __init__(self, domain, protocol):
        """
        Create SP socket.

        :param domain: Socket domain `AF_SP` or `AF_SP_RAW`.
        :param protocol: Type of the socket determining its exact
            semantics.

        .. seealso:: `nn_socket <http://nanomsg.org/v1.0.0/nn_socket.3.html>`_
        """
        self.sock = nanomsg.nn_socket(domain, protocol)
    
    def close(self):
        rc = nanomsg.nn_close(self.sock)
        return errors.convert(rc, rc)
    
    def shutdown(self, who):
        rc = nanomsg.nn_shutdown(self.sock, who)
        return errors.convert(rc, rc)
        
    def getsockopt(self, level, option):
        buf = ffi.new('int*')
        size = ffi.new('size_t*')
        size[0] = 4
        rc = nanomsg.nn_getsockopt(self.sock, level, option, buf, size)
        return errors.convert(rc, lambda: buf[0])
    
    def setsockopt(self, level, option, value):
        if isinstance(value, int):
            buf = ffi.new('int*')
            buf[0] = value
            vlen = 4
        elif isinstance(value, ustr):
            buf = ffi.new('char[%s]' % len(value), value.encode())
            vlen = len(value)
        elif isinstance(value, bytes):
            buf = ffi.new('char[%s]' % len(value), value)
            vlen = len(value)
        else:
            raise TypeError("value must be a int, str or bytes")
        rc = nanomsg.nn_setsockopt(self.sock, level, option, buf, vlen)
        errors.convert(rc)
    
    def bind(self, addr):
        addr = addr.encode() if isinstance(addr, ustr) else addr
        buf = ffi.new('char[]', addr)
        rc = nanomsg.nn_bind(self.sock, buf)
        return errors.convert(rc, rc)
    
    def connect(self, addr):
        addr = addr.encode() if isinstance(addr, ustr) else addr
        buf = ffi.new('char[]', addr)
        rc = nanomsg.nn_connect(self.sock, buf)
        return errors.convert(rc, rc)
    
    def send(self, data, flags=0):
        # Some data types can use a zero-copy buffer creation strategy when
        # paired with new versions of CFFI.  Namely, CFFI 1.8 supports `bytes`
        # types with `from_buffer`, which is about 18% faster.  We try the fast
        # way first and degrade as needed for the platform.
        try:
            buf = ffi.from_buffer(data)
        except TypeError:
            data = data.encode() if isinstance(data, ustr) else data
            buf = ffi.new('char[%i]' % len(data), data)
        rc = nanomsg.nn_send(self.sock, buf, len(data), flags)
        return errors.convert(rc, rc)
    
    def recv(self, flags=0):
        buf = ffi.new('char**')
        rc = nanomsg.nn_recv(self.sock, buf, NN_MSG, flags)
        errors.convert(rc)
        s = ffi.buffer(buf[0], rc)[:]
        nanomsg.nn_freemsg(buf[0])
        return s
        
    def get_statistic(self, statistic):
        rc = nanomsg.nn_get_statistic(self.sock, statistic)
        return errors.convert(rc, rc)
