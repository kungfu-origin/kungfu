from os import environ


def disable():
    [environ.pop(key, None) for key in environ.keys() if key.startswith("KFC_AS_")]


def enable(variant: str):
    environ[f"KFC_AS_{variant.upper()}"] = "on"


def main(**kwargs):
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
