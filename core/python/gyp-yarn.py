import os
import sys
import subprocess

sys.stdout.write(f"node-gyp args: {sys.argv}")
sys.stdout.flush()

cmd = os.path.basename(sys.argv[1])  # strip leading path for gyp on windows
subprocess.run(["yarn", cmd], shell=True)
