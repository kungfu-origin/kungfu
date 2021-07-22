import os
import sys
import subprocess
import platform

argv = list(map(os.path.basename, sys.argv[1:]))  # strip leading path for gyp on windows
cmd = ["yarn", *argv]

sys.stdout.write(f"$ {' '.join(cmd)}{os.linesep}")
sys.stdout.flush()

subprocess.run(cmd, check=True, shell=platform.system() == "Windows")
