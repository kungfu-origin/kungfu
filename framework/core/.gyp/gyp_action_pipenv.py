import os

from gyp_action_lib import Shell
from pathlib import Path


class Pipenv(object):
    SEED_PACKAGES = ["pip", "setuptools", "virtualenv", "wheel"]

    def __init__(self, root_dir=Shell.core_dir):
        self.pipfile_path = os.path.join(root_dir, "Pipfile")
        self.pipfile = Path(self.pipfile_path).read_text().splitlines()

    def find_version(self, package):
        mark = f"{package}="
        for line in self.pipfile:
            s = line.strip().replace(" ", "")
            if s.startswith(mark) and "==" in s:
                return package + s.replace(mark, "").replace('"', "")
        raise NameError(f"No valid semver found for {package} in Pipfile")

    @property
    def seed_packages(self):
        # make sure pipenv install our specified version of the seed packages
        return list(map(self.find_version, Pipenv.SEED_PACKAGES))


Shell.run(["yarn", "pipenv", "--bare", "install"])
Shell.run(["yarn", "pipenv", "install"] + Pipenv().seed_packages)
Shell.touch("Pipfile.lock")
