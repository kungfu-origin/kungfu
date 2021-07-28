import os
import sys
import subprocess
import platform


def extract_argv():
    return list(map(os.path.basename, sys.argv[1:]))


def run(cmd, exit_on_error=True):
    sys.stdout.write(f"$ {' '.join(cmd)}{os.linesep}")
    sys.stdout.flush()
    result = subprocess.run(cmd, check=True, shell=platform.system() == "Windows")
    if result.returncode != 0 and exit_on_error:
        sys.exit(result.returncode)
    return result.returncode
