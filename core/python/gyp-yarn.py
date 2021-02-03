import os
import sys
import subprocess
import platform

sys.stdout.write(f"python gyp args: {sys.argv}\n")
sys.stdout.flush()

cmd = os.path.basename(sys.argv[1])  # strip leading path for gyp on windows
subprocess.run(["yarn", cmd], shell=platform.system() == "Windows")
