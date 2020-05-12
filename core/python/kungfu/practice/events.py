import heapq
import asyncio


class KungfuEventLoop(asyncio.AbstractEventLoop):
    def __init__(self, ctx, hero):
        self._time = 0
        self._running = False
        self._immediate = []
        self._scheduled = []
        self._exception = None
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
            if self._immediate:
                handle = self._immediate[0]
                self._immediate = self._immediate[1:]
            elif self._scheduled:
                handle = heapq.heappop(self._scheduled)
                self._time = handle._when
                handle._scheduled = False  # just for asyncio.TimerHandle debugging?
            else:
                continue
            if not handle._cancelled:
                handle._run()
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

    def call_exceptioneption_handler(self, context):
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
        return self.call_at(self._time + delay, callback, *args, context=context)

    def call_at(self, when, callback, *args, context=None):
        if when < self._time:
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


class ProcessorSharingQueue:
    def __init__(self, service_rate=1, loop=None):
        self._service_rate = service_rate
        self._queue = []
        self._loop = loop if loop else asyncio.get_event_loop()
        self._done = None
        self._work_done = 0
        self._last_time = self._loop.time()

    def process(self, work):
        t = self._advance_clock()
        fut = self._loop.create_future()
        w = work / self._service_rate
        heapq.heappush(self._queue, (self._work_done + w, t, fut))
        if self._done:
            self._done.cancel()
        self._schedule()
        return fut

    def complete(self):
        t = self._advance_clock()
        (_, tstart, fut) = heapq.heappop(self._queue)
        fut.set_result(t - tstart)
        self._schedule()

    def _advance_clock(self):
        t = self._loop.time()
        if self._queue:
            self._work_done += (t - self._last_time) / len(self._queue)
        self._last_time = t
        return t

    def _schedule(self):
        if not self._queue:
            self._done = None
        else:
            w, _, _ = self._queue[0]
            dt = (w - self._work_done) * len(self._queue)
            self._done = self._loop.call_later(dt, self.complete)


class FIFOQueue:
    def __init__(self, service_rate=1, loop=None):
        self._service_rate = service_rate
        self._queue = []
        self._loop = loop if loop else asyncio.get_event_loop()
        self._done = None

    def process(self, work):
        fut = self._loop.create_future()
        w = work / self._service_rate
        self._queue.append((w, fut))
        if not self._done:
            self._done = self._loop.call_later(w, self.complete)
        return fut

    def complete(self):
        w, fut = self._queue[0]
        fut.set_result(w)
        self._queue = self._queue[1:]
        if self._queue:
            w, _ = self._queue[0]
            self._done = self._loop.call_later(w, self.complete)
        else:
            self._done = None
