import gyp_action_lib as lib
import os

from pathlib import Path
from os.path import dirname


def find_version(pipfile, package):
    mark = f"{package}="
    for line in pipfile:
        s = line.strip().replace(' ', '')
        if s.startswith(mark) and "==" in s:
            return package + s.replace(mark, '').replace('"', '')
    raise NameError(f"No valid semver found for {package} in Pipfile")


root_dir = dirname(dirname(lib.__file__))
pipfile_path = os.path.join(root_dir, "Pipfile")

pipfile = Path(pipfile_path).read_text().splitlines()

# make sure pipenv install our specified version of the seed packages
seed_packages = [
    find_version(pipfile, "pip"),
    find_version(pipfile, "setuptools"),
    find_version(pipfile, "wheel"),
]

lib.run(["yarn", "pipenv", "--bare", "install"])
lib.run(["yarn", "pipenv", "install"] + seed_packages)

pipfile_lock = os.path.join(root_dir, "Pipfile.lock")
Path(pipfile_lock).touch()
