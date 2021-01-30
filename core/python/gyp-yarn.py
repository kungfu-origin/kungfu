import os
import sys
import subprocess
import platform

print("node-gyp args:", sys.argv)

cmd = os.path.basename(sys.argv[1])

yarn = 'yarn'
if platform.system() == 'Windows':
    yarn = yarn + '.cmd'

subprocess.run([yarn, cmd])
