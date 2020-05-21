import asyncio
import heapq
import socket
import subprocess
from collections import deque


class KungfuEventLoop(asyncio.AbstractEventLoop):

    def __init__(self, ctx, hero):
        self._time = 0
        self._running = False
        self._immediate = deque()
        self._scheduled = []
        self._exception = None
        self._current = None
        self._ctx = ctx
        self._hero = hero
        self._hero.setup()
        asyncio.set_event_loop(self)

    def get_debug(self):
        return False

    def time(self):
        return self._hero.now()

    def run_forever(self):
        self._running = True
        self._ctx.logger.info('[{:08x}] {} running'.format(self._hero.home.uid, self._hero.home.uname))
        while self._hero.live:
            self._hero.step()

            ready = deque()
            while self._immediate:
                ready.append(self._immediate.popleft())

            if self._scheduled:
                scheduled = []
                while self._scheduled:
                    handle = heapq.heappop(self._scheduled)
                    if handle._when < self._hero.now():
                        self._ctx.logger.info(f'when {handle._when}, now {self._hero.now()}, diff {handle._when - self._hero.now()}')
                        handle._scheduled = False
                        ready.append(handle)
                    else:
                        heapq.heappush(scheduled, handle)
                self._scheduled = scheduled

            while ready:
                self._current = ready.popleft()
                if not self._current._cancelled:
                    self._current._run()
                if self._current:
                    self._immediate.append(self._current)

            if self._exception is not None:
                raise self._exception
        self._hero.on_exit()
        self._ctx.logger.info('[{:08x}] {} done'.format(self._hero.home.uid, self._hero.home.uname))

    def _timer_handle_cancelled(self, handle):
        pass

    def is_running(self):
        return self._hero.live

    def is_closed(self):
        return not self._hero.live

    def stop(self):
        self._running = False

    def close(self):
        self._running = False

    def shutdown_asyncgens(self):
        pass

    def call_exception_handler(self, context):
        self._exception = context.get('exception', None)

    def call_soon(self, callback, *args, context=None):
        handle = asyncio.Handle(callback, args, self)
        self._immediate.append(handle)
        return handle

    def call_later(self, delay, callback, *args, context=None):
        if delay < 0:
            raise Exception("Can't schedule in the past")
        return self.call_at(self._hero.now() + delay * int(1e9), callback, args, context=context)

    def call_at(self, when, callback, *args, context=None):
        if when < self._hero.now():
            raise Exception("Can't schedule in the past")
        handle = asyncio.TimerHandle(when, callback, args, self)
        heapq.heappush(self._scheduled, handle)
        handle._scheduled = True
        self._ctx.logger.info(f'call at when {handle._when}, now {self._hero.now()}, diff {handle._when - self._hero.now()}')
        return handle

    def create_task(self, coro):
        async def wrapper():
            try:
                await coro
            except Exception as e:
                self._exception = e

        return asyncio.Task(wrapper(), loop=self)

    def create_future(self):
        return asyncio.Future(loop=self)

    def set_task_factory(self, factory):
        raise NotImplementedError

    def get_task_factory(self):
        raise NotImplementedError

    def call_soon_threadsafe(self, callback, *args):
        raise NotImplementedError

    def run_until_complete(self, future):
        raise NotImplementedError

    def set_default_executor(self, executor):
        raise NotImplementedError

    async def run_in_executor(self, executor, func, *args):
        raise NotImplementedError

    async def getaddrinfo(self, host, port, *, family=0, type=0, proto=0, flags=0):
        raise NotImplementedError

    async def getnameinfo(self, sockaddr, flags=0):
        raise NotImplementedError

    async def create_connection(self, protocol_factory, host=None, port=None, *,
                                ssl=None, family=0, proto=0, flags=0, sock=None, local_addr=None,
                                server_hostname=None, ssl_handshake_timeout=None, happy_eyeballs_delay=None, interleave=None):
        raise NotImplementedError

    async def create_server(self, protocol_factory, host=None, port=None, *,
                            family=socket.AF_UNSPEC, flags=socket.AI_PASSIVE, sock=None, backlog=100,
                            ssl=None, reuse_address=None, reuse_port=None, ssl_handshake_timeout=None, start_serving=True):
        raise NotImplementedError

    async def create_unix_connection(self, protocol_factory, path=None, *,
                                     ssl=None, sock=None, server_hostname=None, ssl_handshake_timeout=None):
        raise NotImplementedError

    async def create_unix_server(self, protocol_factory, path=None, *,
                                 sock=None, backlog=100, ssl=None, ssl_handshake_timeout=None, start_serving=True):
        raise NotImplementedError

    async def connect_accepted_socket(self, protocol_factory, sock, *, ssl=None, ssl_handshake_timeout=None):
        raise NotImplementedError

    async def sendfile(self, transport, file, offset=0, count=None, *, fallback=True):
        raise NotImplementedError

    async def sock_sendfile(self, sock, file, offset=0, count=None, *, fallback=None):
        raise NotImplementedError

    async def start_tls(self, transport, protocol, sslcontext, *,
                        server_side=False, server_hostname=None, ssl_handshake_timeout=None):
        raise NotImplementedError

    async def create_datagram_endpoint(self, protocol_factory,
                                       local_addr=None, remote_addr=None, *,
                                       family=0, proto=0, flags=0,
                                       reuse_address=None, reuse_port=None,
                                       allow_broadcast=None, sock=None):
        raise NotImplementedError

    async def connect_read_pipe(self, protocol_factory, pipe):
        raise NotImplementedError

    async def connect_write_pipe(self, protocol_factory, pipe):
        raise NotImplementedError

    async def subprocess_shell(self, protocol_factory, cmd, *,
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs):
        raise NotImplementedError

    async def subprocess_exec(self, protocol_factory, *args,
                              stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs):
        raise NotImplementedError

    def add_reader(self, fd, callback, *args):
        raise NotImplementedError

    def remove_reader(self, fd):
        raise NotImplementedError

    def add_writer(self, fd, callback, *args):
        raise NotImplementedError

    def remove_writer(self, fd):
        raise NotImplementedError

    async def sock_recv(self, sock, nbytes):
        raise NotImplementedError

    async def sock_recv_into(self, sock, buf):
        raise NotImplementedError

    async def sock_sendall(self, sock, data):
        raise NotImplementedError

    async def sock_connect(self, sock, address):
        raise NotImplementedError

    async def sock_accept(self, sock):
        raise NotImplementedError

    def add_signal_handler(self, sig, callback, *args):
        raise NotImplementedError

    def remove_signal_handler(self, sig):
        raise NotImplementedError

    def set_exception_handler(self, handler):
        raise NotImplementedError

    def get_exception_handler(self):
        raise NotImplementedError

    def default_exception_handler(self, context):
        raise NotImplementedError

    def set_debug(self, enabled: bool):
        raise NotImplementedError
