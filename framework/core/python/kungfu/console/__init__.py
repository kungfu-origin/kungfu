from kungfu.console import commands, variants

__choices__ = [commands, variants, True]


def alternatives():
    return (__choices__ and __choices__.pop() and __choices__).__reversed__()


def select(modules, **kwargs):
    kwargs.pop("auto_envvar_prefix", None)
    try:
        next(m for m in modules if m.main(auto_envvar_prefix="KF", **kwargs))
        return True
    except StopIteration:
        return False
