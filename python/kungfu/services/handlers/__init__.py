import functools

HANDLERS = dict()
TASKS = dict()

def kfs_handler(handler_type):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        HANDLERS[handler_type] = handler_wrapper
        return kfs_handler
    return register_handler

def kfs_handle(handler_type, *args, **kwargs):
    return HANDLERS[handler_type](*args, **kwargs)

def kfs_task(func):
    @functools.wraps(func)
    def task_wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    TASKS[func.__name__] = task_wrapper
    print('register task ', func.__name__)
    return task_wrapper

def kfs_run_tasks(*args, **kwargs):
    for task_name in TASKS:
        TASKS[task_name](*args, **kwargs)
