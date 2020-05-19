import heapq
import asyncio
from collections import deque


# https://gist.github.com/damonjw/35aac361ca5d313ee9bf79e00261f4ea


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
            while self._scheduled:
                handle = heapq.heappop(self._scheduled)
                handle._scheduled = False  # just for asyncio.TimerHandle debugging?
                ready.append(handle)
            while ready:
                self._current = ready.popleft()
                if not self._current._cancelled:
                    self._current._run()
                if self._current:
                    self._immediate.append(self._current)
            if self._exception is not None:
                raise self._exception
        self._ctx.logger.info('[{:08x}] {} done'.format(self._hero.home.uid, self._hero.home.uname))

    def run_until_complete(self, future):
        raise NotImplementedError

    def _timer_handle_cancelled(self, handle):
        # We could remove the handle from _scheduled, but instead we'll just skip
        # over it in the "run_forever" method.
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
        # If there is any exception in a callback, this method gets called.
        # I'll store the exception in self._exception, so that the main simulation loop picks it up.
        self._exception = context.get('exception', None)

    # Methods for scheduling jobs.
    #
    # If the job is a coroutine, the end-user should call asyncio.ensure_future(coro()).
    # The asyncio machinery will invoke loop.create_task(). Asyncio will then
    # run the coroutine in pieces, breaking it apart at async/await points, and every time it
    # will construct an appropriate callback and call loop.call_soon(cb).
    #
    # If the job is a plain function, the end-user should call one of the loop.call_*()
    # methods directly.

    def call_soon(self, callback, *args, context=None):
        handle = asyncio.Handle(callback, args, self)
        self._immediate.append(handle)
        return handle

    def call_later(self, delay, callback, *args, context=None):
        if delay < 0:
            raise Exception("Can't schedule in the past")
        return self.call_at(self._hero.now() + delay, callback, *args, context=context)

    def call_at(self, when, callback, *args, context=None):
        if when < self._hero.now():
            raise Exception("Can't schedule in the past")
        handle = asyncio.TimerHandle(when, callback, args, self)
        heapq.heappush(self._scheduled, handle)
        handle._scheduled = True  # perhaps just for debugging in asyncio.TimerHandle?
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
