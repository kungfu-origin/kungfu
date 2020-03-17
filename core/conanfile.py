import os
import sys
import json
import shutil
import getpass
import platform
import subprocess
import datetime
from conans import ConanFile
from conans import tools


with open(os.path.join('package.json'), 'r') as package_json_file:
    package_json = json.load(package_json_file)


class KungfuCoreConan(ConanFile):
    name = package_json['name']
    version = package_json['version']
    generators = "cmake"
    requires = [
        "fmt/6.1.2",
        "spdlog/1.5.0",
        "rxcpp/4.1.0@bincrafters/stable",
    ]
    settings = {
        "build_type": ["Release", "Debug"],
        "arch": ["x86_64"],
    }
    options = {
        "log_level": ["trace", "debug", "info", "warning", "error", "critical"],
        "arch": ["x64"],
        "js_runtime": ["electron", "node"],
        "node_version": ["10.16.0"],
        "electron_version": ["4.2.11"]
    }
    default_options = {
        "log_level": "info",
        "arch": "x64",
        "js_runtime": "electron",
        "node_version": "10.16.0",
        "electron_version": "4.2.11"
    }

    def build(self):
        build_type = self._get_build_type()
        self._gen_build_info(build_type)
        self._run_cmake_js(build_type, 'build')
        self._show_build_info(build_type)

    def package(self):
        build_type = self._get_build_type()
        self._run_pyinstaller(build_type)
        self._run_setuptools(build_type)
        self._show_build_info(build_type)

    def _get_build_type(self):
        build_type = str(self.settings.build_type)
        os.environ['CMAKE_BUILD_TYPE'] = build_type
        return build_type

    def _get_node_version(self):
        return str(self.options.electron_version) if self.options.js_runtime == 'electron' else str(self.options.node_version)

    def _gen_build_info(self, build_type):
        now = datetime.datetime.now()
        build_date = now.strftime('%Y%m%d%H%M%S')

        git = tools.Git()
        git_revision = git.get_revision()[:8]

        build_version = f'{self.version}.{build_date}.{git_revision}' if tools.Version(self.version).prerelease else self.version

        build_info = {
            'version': build_version,
            'pythonVersion': platform.python_version(),
            'git': {
                'tag': git.get_tag(),
                'branch': git.get_branch(),
                'revision': git.get_revision(),
                'pristine': git.is_pristine()
            },
            'build': {
                'user': getpass.getuser(),
                'osVersion': tools.os_info.os_version,
                'timestamp': now.strftime('%Y/%m/%d %H:%M:%S')
            }
        }
        tools.mkdir(build_type)
        with open(os.path.join(build_type, 'kungfubuildinfo.json'), 'w') as output:
            json.dump(build_info, output, indent=2)
        self.output.success(f'build version {build_version}')

    def _show_build_info(self, build_type):
        with open(os.path.join(build_type, 'kungfubuildinfo.json'), 'r') as build_info_file:
            build_info = json.load(build_info_file)
            build_version = build_info['version']
            self.output.success(f'build version {build_version}')

    def _build_cmake_js_cmd(self, cmd):
        spdlog_levels = {
            'trace': 'SPDLOG_LEVEL_TRACE',
            'debug': 'SPDLOG_LEVEL_DEBUG',
            'info': 'SPDLOG_LEVEL_INFO',
            'warning': 'SPDLOG_LEVEL_WARN',
            'error': 'SPDLOG_LEVEL_ERROR',
            'critical': 'SPDLOG_LEVEL_CRITICAL'
        }
        loglevel = spdlog_levels[str(self.options.log_level)]
        python_path = subprocess.Popen(['pipenv', '--py'], stdout=subprocess.PIPE).stdout.read().decode().strip()

        cmake_js_cmd = [
            tools.which('yarn'),
            'cmake-js',
            '--debug' if self.settings.build_type == 'Debug' else '',
            '--arch', str(self.options.arch),
            '--runtime', str(self.options.js_runtime),
            '--runtime-version', self._get_node_version(),
            '--CDPYTHON_EXECUTABLE=' + python_path,
            '--CDSPDLOG_LOG_LEVEL_COMPILE=' + loglevel
        ]

        if tools.detected_os() == 'Windows':
            return cmake_js_cmd + ['--toolset', 'host=' + str(self.options.arch),
                                   '--CDCMAKE_GENERATOR_PLATFORM=' + str(self.options.arch), cmd]
        else:
            return cmake_js_cmd + [cmd]

    def _run_cmake_js(self, build_type, cmd):
        rc = subprocess.Popen(self._build_cmake_js_cmd(cmd)).wait()
        if rc != 0:
            self.output.error(f'cmake-js {cmd} failed')
            sys.exit(rc)
        self.output.success(f'cmake-js {cmd} done')

    def _run_pyinstaller(self, build_type):
        with tools.chdir(os.path.pardir):
            rc = subprocess.Popen(['pipenv', 'run', 'pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-conf.spec']).wait()
            if tools.detected_os() == 'Macos' and os.path.exists('build/kfc/.Python'):
                os.chdir(os.path.join('build', 'kfc'))
                os.rename('.Python', 'Python')
                os.symlink('Python', '.Python')
            if tools.detected_os() == 'Windows' and os.path.exists(os.path.join('build', build_type, 'sqlite3.dll')):
                shutil.copy2(os.path.join('build', build_type, 'sqlite3.dll'), os.path.join('build', 'kfc'))
            if rc != 0:
                self.output.error('PyInstaller failed')
                sys.exit(rc)
        self.output.success('PyInstaller done')

    def _run_setuptools(self, build_type):
        python_build_dir = 'python'
        tools.rmdir(python_build_dir)
        shutil.copytree(build_type, python_build_dir, ignore=shutil.ignore_patterns('node*'))
        shutil.copytree(os.path.join(os.pardir, 'python', 'kungfu'), os.path.join(python_build_dir, 'kungfu'))
        shutil.copy2(os.path.join(os.pardir, 'python', 'setup.py'), python_build_dir)

        with tools.chdir('python'):
            rc = subprocess.Popen(['pipenv', 'run', 'python', 'setup.py', 'bdist_wheel']).wait()
            if rc != 0:
                self.output.error('setuptools failed')
                sys.exit(rc)
        self.output.success('setuptools done')
