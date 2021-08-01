def pick(**kwargs):
    from distutils.util import strtobool
    from os import environ
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
