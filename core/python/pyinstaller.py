import platform
import os

osname = platform.system()

if 'debug' in os.environ['CMAKEJSOPT']:
    os.environ['CMAKEBUILDTYPE'] = 'Debug'
else:
    os.environ['CMAKEBUILDTYPE'] = 'Release'

if osname == 'Darwin':
    os.system(" ".join(["pyinstaller", "--clean", "-y", "--distpath=build", "python/kfc-unix.spec"]))
    os.chdir('build/kfc')
    os.rename('.Python', 'Python')
    os.symlink('Python', '.Python')
if osname == 'Linux':
    os.system(" ".join(["pyinstaller", "--clean", "-y", "--distpath=build", "python/kfc-unix.spec"]))
if osname == 'Windows':
    os.system(" ".join(["pyinstaller", "--clean", "-y", r"--distpath=build", r"python\kfc-win.spec"]))
