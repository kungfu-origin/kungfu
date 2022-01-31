import gyp_action_lib as lib
import os

from pathlib import Path
from os.path import dirname

lib.run(["yarn", "poetry", "lock", "-n", "-q", "--no-update"])
lib.run(["yarn", "poetry", "install", "-n", "-q", "--no-root"])

poetry_lock = os.path.join(dirname(dirname(lib.__file__)), "poetry.lock")
Path(poetry_lock).touch()
