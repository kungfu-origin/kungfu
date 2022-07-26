import os
import sys
import subprocess
import platform

from pathlib import Path
from os.path import dirname, join


class Shell:
    core_dir = dirname(dirname(__file__))

    @staticmethod
    def extract_argv():
        return list(map(os.path.basename, sys.argv[1:]))

    @staticmethod
    def run(cmd, exit_on_error=True):
        sys.stdout.write(f"$ {' '.join(cmd)}{os.linesep}")
        sys.stdout.flush()
        result = subprocess.run(cmd, check=True, shell=platform.system() == "Windows")
        if result.returncode != 0 and exit_on_error:
            sys.exit(result.returncode)
        sys.stdout.write(os.linesep)
        sys.stdout.flush()
        return result.returncode

    @staticmethod
    def touch(filename, cwd=core_dir):
        Path(join(cwd, filename)).touch()
