import functools

HANDLERS = dict()
TASKS = dict()

def on(request_path):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        HANDLERS[request_path] = handler_wrapper
        return on
    return register_handler

def handle(request_path, *args, **kwargs):
    if request_path not in HANDLERS:
        args[0]._logger.error("invalid request path %s", request_path)
    return HANDLERS[request_path](*args, **kwargs)

def task(func):
    @functools.wraps(func)
    def task_wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    TASKS[func.__name__] = task_wrapper
    return task_wrapper

def run_tasks(*args, **kwargs):
    for task_name in TASKS:
        TASKS[task_name](*args, **kwargs)

from . import system
from . import paged
from . import calendar
