import heapq
import asyncio
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
                    if handle._when > self._hero.now():
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
        self._ctx.logger.info('[{:08x}] {} done'.format(self._hero.home.uid, self._hero.home.uname))

    def run_until_complete(self, future):
        raise NotImplementedError

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
        self._ctx.logger.info('call soon')
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
        handle._scheduled = True
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
