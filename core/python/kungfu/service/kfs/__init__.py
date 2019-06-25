import functools

HANDLERS = dict()
TASKS = dict()

def on(msg_type):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        HANDLERS[msg_type] = handler_wrapper
        return on
    return register_handler

def handle(msg_type, *args, **kwargs):
    if msg_type not in HANDLERS:
        args[0].logger.error("invalid request path %s", msg_type)
    return HANDLERS[msg_type](*args, **kwargs)

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
from . import calendar
