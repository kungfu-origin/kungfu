import functools

HANDLERS = dict()
TASKS = dict()

def kfs_on(request_path):
    def register_handler(func):
        @functools.wraps(func)
        def handler_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        HANDLERS[request_path] = handler_wrapper
        return kfs_on
    return register_handler

def kfs_handle(request_path, *args, **kwargs):
    return HANDLERS[request_path](*args, **kwargs)

def task(func):
    @functools.wraps(func)
    def task_wrapper(*args, **kwargs):
        return func(*args, **kwargs)
    TASKS[func.__name__] = task_wrapper
    return task_wrapper

def kfs_run_tasks(*args, **kwargs):
    for task_name in TASKS:
        TASKS[task_name](*args, **kwargs)
