import gyp_action_lib as lib
import os

from pathlib import Path
from os.path import dirname

lib.run(["yarn", "pipenv", "--bare", "install"])

pipfile_lock = os.path.join(dirname(dirname(lib.__file__)), "Pipfile.lock")
Path(pipfile_lock).touch()
