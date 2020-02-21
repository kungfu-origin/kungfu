import os
import sys
import subprocess
import datetime
from conans import ConanFile
from conans import tools


class KungfuCoreConan(ConanFile):
    name = "kungfu-core"
    version = "2.2.0-dev"
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
        "runtime": ["electron", "node"],
        "node_version": ["10.16.0"],
        "electron_version": ["4.2.11"]
    }
    default_options = {
        "log_level": "info",
        "arch": "x64",
        "runtime": "electron",
        "node_version": "10.16.0",
        "electron_version": "4.2.11"
    }

    def build(self):
        cmd = self._build_cmake_js_cmd('build')
        sys.exit(subprocess.Popen(cmd).wait())

    def package(self):
        build_type = str(self.settings.build_type)
        os.environ['CMAKE_BUILD_TYPE'] = build_type

        git = tools.Git()
        git_revision = git.get_revision()[:8]
        build_date = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
        build_version = f'{self.version}.{build_date}.{git_revision}' if tools.Version(self.version).prerelease else self.version
        with open(os.path.join(os.getcwd(), build_type, 'version.info'), 'w') as version_file:
            version_file.write(build_version)

        with tools.chdir('..'):
            rc = subprocess.Popen(['pipenv', 'run', 'pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-conf.spec']).wait()
            if tools.detected_os() == 'Macos' and os.path.exists('build/kfc/.Python'):
                os.chdir('build/kfc')
                os.rename('.Python', 'Python')
                os.symlink('Python', '.Python')
            if rc != 0:
                self.output.error('PyInstaller failed')
                sys.exit(rc)

        self.output.success('PyInstaller done')

        setup_py = os.path.abspath(os.path.join('..', 'python', 'setup.py'))
        setup_dir = 'setuptools'
        tools.rmdir(setup_dir)
        tools.mkdir(setup_dir)
        with tools.chdir(setup_dir):
            rc = subprocess.Popen(['pipenv', 'run', 'python', setup_py, 'bdist_wheel']).wait()
            if rc != 0:
                self.output.error('setuptools failed')
                sys.exit(rc)

        self.output.success('setuptools done')

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
        runtime_version = str(self.options.electron_version) if self.options.runtime == 'electron' else str(self.options.node_version)

        cmake_js_cmd = [tools.which('yarn'), 'cmake-js', '--debug' if self.settings.build_type == 'Debug' else '',
                        '--arch', str(self.options.arch),
                        '--runtime', str(self.options.runtime),
                        '--runtime-version', runtime_version,
                        '--CDPYTHON_EXECUTABLE=' + python_path,
                        '--CDSPDLOG_LOG_LEVEL_COMPILE=' + loglevel]

        if tools.detected_os() == 'Windows':
            return cmake_js_cmd + ['--toolset', 'host=' + str(self.options.arch),
                                   '--CDCMAKE_GENERATOR_PLATFORM=' + str(self.options.arch), cmd]
        else:
            return cmake_js_cmd + [cmd]
