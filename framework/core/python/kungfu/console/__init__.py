def select(runners, **kwargs):
    kwargs.pop("auto_envvar_prefix", None)
    try:
        next(r for r in runners if r(auto_envvar_prefix="KF", **kwargs))
        return True
    except StopIteration:
        return False
