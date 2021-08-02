from os import environ


def enable(variant: str):
    for key in environ.keys():
        if key.startswith("KFC_AS_"):
            environ.pop(key, None)
    environ[f"KFC_AS_{variant.upper()}"] = "on"


def pick(**kwargs):
    from distutils.util import strtobool
    from .__registry__ import env

    def try_run(key):
        if key in environ and bool(strtobool(environ[key])):
            env[key](**kwargs)
            return True
        return False

    try:
        next(k for k in env if try_run(k))
        return True
    except StopIteration:
        return False
