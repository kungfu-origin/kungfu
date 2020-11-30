import os
import sys
import json
import atexit
import psutil
import shutil
import getpass
import platform
import datetime
import subprocess
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
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "log_level": ["trace", "debug", "info", "warning", "error", "critical"],
        "arch": ["x64"],
        "js_runtime": ["electron", "node"],
        "node_version": ["10.16.0"],
        "electron_version": ["4.2.11"]
    }
    default_options = {
        "fmt:header_only": "True",
        "spdlog:header_only": "True",
        "log_level": "info",
        "arch": "x64",
        "js_runtime": "electron",
        "node_version": "10.16.0",
        "electron_version": "4.2.11"
    }
    cpp_files_extensions = ['.h', '.hpp', '.hxx', '.cpp', '.c', '.cc', '.cxx']
    conanfile_dir = os.path.dirname(os.path.realpath(__file__))
    cpp_dir = os.path.join(conanfile_dir, "cpp")
    ext_dir = os.path.join(conanfile_dir, "extensions")
    build_dir = os.path.join(conanfile_dir, 'build')
    build_python_dir = os.path.join(build_dir, 'python')
    build_extensions_dir = os.path.join(build_dir, 'build_extensions')

    def source(self):
        """Performs clang-format on all C++ files"""
        if tools.which('clang-format') is not None:
            self.__clang_format(self.cpp_dir, self.cpp_files_extensions)
            self.__clang_format(os.path.join(self.ext_dir, 'xtp', 'src'), self.cpp_files_extensions)
            self.__clang_format(os.path.join(self.ext_dir, 'ctp', 'src'), self.cpp_files_extensions)
        else:
            self.output.warn('clang-format not installed')

    def configure(self):
        if tools.detected_os() != 'Windows':
            self.settings.compiler.libcxx = "libstdc++"

    def build(self):
        build_type = self.__get_build_type()
        self.__gen_build_info(build_type)
        self.__run_cmake_js(build_type, 'build')
        self.__show_build_info(build_type)

    def imports(self):
        self.copy('*', src='include', dst='include')

    def package(self):
        build_type = self.__get_build_type()
        self.__run_pyinstaller(build_type)
        self.__run_setuptools(build_type)
        self.__show_build_info(build_type)

    def __get_build_type(self):
        build_type = str(self.settings.build_type)
        os.environ['CMAKE_BUILD_TYPE'] = build_type
        return build_type

    def __get_node_version(self):
        return str(self.options.electron_version) if self.options.js_runtime == 'electron' else str(self.options.node_version)

    def __list_files(self, source_dir, extensions):
        found_files = []
        for path, subdirs, files in os.walk(source_dir):
            for file in files:
                extension = os.path.splitext(file)[1]
                for ext in extensions:
                    if ext == extension:
                        file_path = os.path.join(path, file)
                        found_files.append(file_path)
        return found_files

    def __clang_format(self, source_dir, extensions):
        files = self.__list_files(source_dir, extensions)
        processes = [psutil.Popen(['clang-format', '--verbose', '-style=file', '-i', file]) for file in files]

        def cleanup():
            map(lambda p: p.kill(), [p for p in processes if p.is_running()])

        atexit.register(cleanup)

        for process in processes:
            try:
                rc = process.wait(30)
                if rc is not None and rc != 0:
                    self.output.error(f'clang-format failed rc={rc}')
                    self.output.error(' '.join(process.cmdline()))
            except psutil.TimeoutExpired:
                self.output.error(f'clang-format timeout')
                self.output.error(' '.join(process.cmdline()))
                process.kill()
            except Exception as e:
                self.output.error(f'clang-format failed with error {e}')
                self.output.error(' '.join(process.cmdline()))
                process.kill()

    def __gen_build_info(self, build_type):
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

    def __show_build_info(self, build_type):
        with open(os.path.join(build_type, 'kungfubuildinfo.json'), 'r') as build_info_file:
            build_info = json.load(build_info_file)
            build_version = build_info['version']
            self.output.success(f'build version {build_version}')

    def __build_cmake_js_cmd(self, cmd):
        spdlog_levels = {
            'trace': 'SPDLOG_LEVEL_TRACE',
            'debug': 'SPDLOG_LEVEL_DEBUG',
            'info': 'SPDLOG_LEVEL_INFO',
            'warning': 'SPDLOG_LEVEL_WARN',
            'error': 'SPDLOG_LEVEL_ERROR',
            'critical': 'SPDLOG_LEVEL_CRITICAL'
        }
        loglevel = spdlog_levels[str(self.options.log_level)]
        python_path = psutil.Popen(['pipenv', '--py'], stdout=subprocess.PIPE).stdout.read().decode().strip()

        build_option = ['--toolset', 'host=' + str(self.options.arch)] if tools.detected_os() == 'Windows' else []
        debug_option = ['--debug'] if self.settings.build_type == 'Debug' else []

        return [
            tools.which('yarn'),
            'cmake-js',
            '--arch', str(self.options.arch),
            '--runtime', str(self.options.js_runtime),
            '--runtime-version', self.__get_node_version(),
            f'--CDPYTHON_EXECUTABLE={python_path}',
            f'--CDSPDLOG_LOG_LEVEL_COMPILE={loglevel}',
            f'--CDCMAKE_BUILD_PARALLEL_LEVEL={os.cpu_count()}'
        ] + build_option + debug_option + [cmd]

    def __run_cmake_js(self, build_type, cmd):
        [os.environ.pop(env_key) for env_key in os.environ if env_key.startswith('npm_') or env_key.startswith('NPM_')] # workaround for msvc
        tools.rmdir(self.build_extensions_dir)
        rc = psutil.Popen(self.__build_cmake_js_cmd(cmd)).wait()
        if rc != 0:
            self.output.error(f'cmake-js {cmd} failed')
            sys.exit(rc)
        self.output.success(f'cmake-js {cmd} done')

    def __run_pyinstaller(self, build_type):
        with tools.chdir(os.path.pardir):
            rc = psutil.Popen(['pipenv', 'run', 'pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-conf.spec']).wait()
            if tools.detected_os() == 'Macos' and os.path.exists('build/kfc/.Python'):
                os.chdir(os.path.join('build', 'kfc'))
                os.rename('.Python', 'Python')
                os.symlink('Python', '.Python')
            if rc != 0:
                self.output.error('PyInstaller failed')
                sys.exit(rc)
        self.output.success('PyInstaller done')

    def __run_setuptools(self, build_type):
        tools.rmdir(self.build_python_dir)
        shutil.copytree(build_type, self.build_python_dir, ignore=shutil.ignore_patterns('node*'))
        shutil.copytree(os.path.join(os.pardir, 'python', 'kungfu'), os.path.join(self.build_python_dir, 'kungfu'))
        shutil.copytree(os.path.join(os.pardir, 'python', 'kungfu_extensions'), os.path.join(self.build_python_dir, 'kungfu_extensions'))
        shutil.copy2(os.path.join(os.pardir, 'python', 'setup.py'), self.build_python_dir)

        with tools.chdir('python'):
            rc = psutil.Popen(['pipenv', 'run', 'python', 'setup.py', 'bdist_wheel']).wait()
            if rc != 0:
                self.output.error('setuptools failed')
                sys.exit(rc)
        self.output.success('setuptools done')
