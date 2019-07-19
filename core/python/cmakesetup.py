import os
import subprocess
import platform

python_path = subprocess.Popen(["pipenv", "--py"], stdout=subprocess.PIPE).stdout.read().strip()

npm_path = "npm"
if platform.system() == 'Windows':
    python_path = python_path.decode('utf8')
    for line in subprocess.Popen(["where", "npm"], stdout=subprocess.PIPE).stdout.readlines():
        path = line.decode('utf8').strip()
        if path.endswith('.cmd'):
            npm_path = path

spdlog_levels = {
    'trace':        'SPDLOG_LEVEL_TRACE',
    'debug':        'SPDLOG_LEVEL_DEBUG',
    'info':         'SPDLOG_LEVEL_INFO',
    'warn':         'SPDLOG_LEVEL_WARN',
    'error':        'SPDLOG_LEVEL_ERROR',
    'critical':     'SPDLOG_LEVEL_CRITICAL'
}
loglevel_env = os.environ['KF_LOG_LEVEL_COMPILE']
loglevel = spdlog_levels[loglevel_env] if loglevel_env else spdlog_levels['warn']

subprocess.Popen([npm_path, "config", "set", "cmake_PYTHON_EXECUTABLE", python_path])
subprocess.Popen([npm_path, "config", "set", "cmake_SPDLOG_LOG_LEVEL_COMPILE", loglevel])
subprocess.Popen([npm_path, "config", "set", "cmake_CMAKE_GENERATOR_PLATFORM", 'x64'])
