import subprocess
import platform

python_path = subprocess.Popen(["pipenv", "--py"], stdout=subprocess.PIPE).stdout.read().strip()

yarn_path = "yarn"
if platform.system() == 'Windows':
    python_path = python_path.decode('utf8')
    for line in subprocess.Popen(["where", "yarn"], stdout=subprocess.PIPE).stdout.readlines():
        path = line.decode('utf8').strip()
        if path.endswith('.cmd'):
            yarn_path = path

subprocess.Popen([yarn_path, "config", "set", "cmake_PYTHON_EXECUTABLE", python_path])
