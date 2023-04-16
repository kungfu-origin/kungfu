# SPDX-License-Identifier: Apache-2.0

import json
import getpass
import os
import pathlib
import platform
import datetime
import shutil
import stat
import subprocess
import sys

from conans import ConanFile
from conans import tools
from conans.errors import ConanException
from distutils import sysconfig
from os import environ
from os import path

with open(path.join("package.json"), "r") as package_json_file:
    package_json = json.load(package_json_file)


class KungfuCoreConan(ConanFile):
    name = "kungfu-core"
    version = package_json["version"]
    generators = "cmake"
    requires = [
        "fmt/8.1.1",
        "nlohmann_json/3.11.2",
        "nng/1.5.2",
        "rxcpp/4.1.1",
        "sqlite3/3.39.2",
        # "sqlite_orm/1.7.1",
        "spdlog/1.10.0",
        "tabulate/1.4",
    ]
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "log_level": ["trace", "debug", "info", "warning", "error", "critical"],
        "arch": ["x64"],
        "freezer": ["nuitka", "pyinstaller"],
        "node_version": "ANY",
        "electron_version": "ANY",
        "vs_toolset": ["auto", "ClangCL"],
        "with_yarn": [True, False],
    }
    default_options = {
        "fmt:header_only": True,
        "spdlog:header_only": True,
        "spdlog:shared": False,
        "sqlite3:enable_column_metadata": True,
        "sqlite3:enable_json1": True,
        "sqlite3:enable_preupdate_hook": True,
        "sqlite3:enable_dbstat_vtab": True,
        "sqlite3:shared": False,
        "nng:http": False,
        "log_level": "info",
        "arch": "x64",
        "freezer": "pyinstaller",
        "node_version": "ANY",
        "electron_version": "ANY",
        # clang has a known issue:
        # https://developercommunity.visualstudio.com/t/msbuild-doesnt-give-delayload-flags-to-linker-when/1595015
        "vs_toolset": "auto"
        if "CONAN_VS_TOOLSET" not in environ
        else environ["CONAN_VS_TOOLSET"],
        "with_yarn": False,
    }
    gyp_call = "NODE_GYP_RUN" in os.environ
    exports = "package.json"
    exports_sources = "src/*", "package.json", "CMakeLists.txt", ".cmake/*"
    conanfile_dir = path.dirname(path.realpath(__file__))
    pyi_hooks_dir = path.join(conanfile_dir, "src", "python", "pyi-hooks")
    build_info_file = "kungfubuildinfo.json"
    build_dir = path.join(conanfile_dir, "build")
    dist_dir = path.join(conanfile_dir, "dist")
    kfc_dir = path.join(dist_dir, "kfc")
    kfs_dir = path.join(dist_dir, "kfs")

    def configure(self):
        if tools.detected_os() != "Windows":
            self.settings.compiler.libcxx = "libstdc++"
        else:
            toolset = self.__get_toolset()
            if toolset != "auto":
                self.settings.compiler.toolset = toolset

    def generate(self):
        """Updates mtime of lock files for node-gyp sake"""
        if self.gyp_call:
            self.__touch_lockfile()

    def imports(self):
        python_inc_src = sysconfig.get_python_inc(plat_specific=True)
        python_inc_dst = (
            "include"
            if path.basename(python_inc_src) == "include"
            else path.join("include", path.basename(python_inc_src))
        )
        self.copy("*", src=python_inc_src, dst=python_inc_dst)
        self.copy("*", src="include", dst="include")

    def build(self):
        build_type = self.__get_build_type()
        self.__clean_build_info(build_type)
        self.__run_build(build_type, "node")
        self.__run_build(build_type, "electron")
        self.__gen_build_info(build_type)
        self.__show_build_info(build_type)

    def package(self):
        build_type = self.__get_build_type()
        if self.gyp_call:
            self.__clean_dist_dir()
            self.__run_freeze(build_type)
            self.__show_build_info(build_type)
        else:
            self.copy("*", dst="include", src="src/include")
            self.copy("*", dst="lib", src=build_type)
            self.copy("*", dst="bin", src=path.join("src", "libkungfu", build_type))

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "kungfu"
        self.cpp_info.names["cmake_find_package_multi"] = "kungfu"
        self.cpp_info.libs = ["kungfu"]

    def __get_build_type(self):
        build_type = str(self.settings.build_type)
        os.environ["CMAKE_BUILD_TYPE"] = build_type
        return build_type

    def __get_toolset(self):
        return str(self.options.vs_toolset)

    def __get_node_version(self, runtime):
        return (
            str(self.options.electron_version)
            if runtime == "electron"
            else str(self.options.node_version)
        )

    def __get_build_info_path(self, build_type):
        return path.join(self.build_dir, build_type, self.build_info_file)

    def __touch_lockfile(self):
        conan_lock = path.join(self.build_dir, "conan.lock")
        pathlib.Path(conan_lock).touch()

    def __clean_build_info(self, build_type):
        build_info_path = self.__get_build_info_path(build_type)
        if path.exists(build_info_path):
            os.remove(build_info_path)
            self.output.info("Deleted kungfubuildinfo.json")

    def __clean_dist_dir(self):
        if path.exists(self.dist_dir):

            def redo_with_write(redo_func, path, err):
                os.chmod(path, stat.S_IWRITE)
                redo_func(path)

            shutil.rmtree(self.dist_dir, onerror=redo_with_write)
            self.output.info("Deleted dist directory")

    def __gen_build_info(self, build_type):
        now = datetime.datetime.now()
        build_info = {
            "version": self.version,
            "pythonVersion": platform.python_version(),
            "build": {
                "user": getpass.getuser(),
                "osVersion": tools.os_info.os_version,
                "timestamp": now.strftime("%Y/%m/%d %H:%M:%S"),
            },
        }

        try:
            git = tools.Git()
            build_info["git"] = {
                "tag": git.get_tag(),
                "branch": git.get_branch(),
                "revision": git.get_revision(),
                "pristine": git.is_pristine(),
            }
        except ConanException:
            pass

        tools.mkdir(path.join(self.build_dir, build_type))
        with open(self.__get_build_info_path(build_type), "w") as output:
            json.dump(build_info, output, indent=2)

    def __show_build_info(self, build_type):
        with open(self.__get_build_info_path(build_type), "r") as build_info_file:
            build_info = json.load(build_info_file)
            build_version = build_info["version"]
            self.output.success(f"build version {build_version}")

    def __enable_modules(self, runtime):
        modules = {
            "libkungfu": True,
            "kungfu_node": (tools.detected_os() != "Windows")
            or (runtime == "electron"),
            "pykungfu": runtime == "node",
        }

        def switch(module):
            environ_key = f"KUNGFU_BUILD_SKIP_{module.upper()}"
            if not modules[module]:
                environ[environ_key] = "on"
            else:
                environ.pop(environ_key, None)

        [switch(key) for key in modules.keys()]

    def __run_build(self, build_type, runtime):
        if f"KUNGFU_BUILD_SKIP_RUNTIME_{runtime.upper()}" in environ:
            self.output.warn(f"disabled build for runtime {runtime}")
            return
        toolset = self.__get_toolset()
        parallel_opt = (
            []
            if tools.detected_os() == "Windows"
            else ["--", "-j", f"{os.cpu_count()}"]
        )
        self.__enable_modules(runtime)
        if str(self.options.with_yarn) == "True":
            self.__run_cmake_js(build_type, "configure", runtime, toolset)
            self.__run_cmake_js(build_type, "build", runtime, toolset)
        elif runtime == "node":
            environ["KUNGFU_BUILD_SKIP_KUNGFU_NODE"] = "on"
            environ["KUNGFU_BUILD_SKIP_PYKUNGFU"] = "on"
            self.__run_cmake(
                "-S",
                ".." if self.gyp_call else ".",
                "-B",
                "../build" if self.gyp_call else ".",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DSPDLOG_LOG_LEVEL_COMPILE=trace",
            )
            self.__run_cmake("--build", ".", "--config", "Release", *parallel_opt)

    def __run_cmake(self, *args):
        rc = subprocess.Popen([tools.which("cmake"), *args]).wait()
        if rc != 0:
            self.output.error(f"cmake {args} failed with return code {rc}")
            sys.exit(rc)

    def __run_cmake_js(self, build_type, cmd, runtime, toolset):
        [
            os.environ.pop(env_key)
            for env_key in os.environ
            if env_key.upper().startswith("NPM_")
        ]  # workaround for msvc
        self.__run_yarn(*self.__build_cmake_js_cmd(build_type, cmd, runtime, toolset))
        self.output.success(f"cmake-js {cmd} done")

    def __run_yarn(self, *args):
        yarn = "yarn" if tools.detected_os() != "Windows" else "yarn.cmd"
        rc = subprocess.Popen([tools.which(yarn), *args]).wait()
        if rc != 0:
            self.output.error(f"yarn {args} failed with return code {rc}")
            sys.exit(rc)

    def __build_cmake_js_cmd(self, build_type, cmd, runtime, toolset):
        spdlog_levels = {
            "trace": "SPDLOG_LEVEL_TRACE",
            "debug": "SPDLOG_LEVEL_DEBUG",
            "info": "SPDLOG_LEVEL_INFO",
            "warning": "SPDLOG_LEVEL_WARN",
            "error": "SPDLOG_LEVEL_ERROR",
            "critical": "SPDLOG_LEVEL_CRITICAL",
        }
        log_level = spdlog_levels[str(self.options.log_level)]

        parallel_level = os.cpu_count()

        python_path = (
            subprocess.Popen(["pipenv", "--py"], stdout=subprocess.PIPE)
            .stdout.read()
            .decode()
            .strip()
        )

        toolset_option = ["--toolset", toolset] if toolset != "auto" else []

        build_option = (
            toolset_option + ["--platform", str(self.options.arch)]
            if tools.detected_os() == "Windows"
            else ["--parallel", str(parallel_level)]
        )

        debug_option = ["--debug"] if build_type == "Debug" else []

        return (
            [
                "cmake-js",
                "--arch",
                str(self.options.arch),
                "--runtime",
                runtime,
                "--runtime-version",
                self.__get_node_version(runtime),
                f"--CDPYTHON_EXECUTABLE={python_path}",
                f"--CDSPDLOG_LOG_LEVEL_COMPILE={log_level}",
                f"--CDCMAKE_BUILD_PARALLEL_LEVEL={parallel_level}",
            ]
            + build_option
            + debug_option
            + [cmd]
        )

    def __run_pyinstaller(self, build_type):
        pathlib.Path(self.__get_build_info_path(build_type)).touch()
        with tools.chdir(path.pardir):
            from PyInstaller import __main__ as freezer

            freezer.run(
                [
                    f"--workpath={path.join('.', 'build')}",
                    f"--distpath={path.join('.', 'dist')}",
                    "--clean",
                    "--noconfirm",
                    path.join(".", "src", "python", "kfc.spec"),
                ]
            )

        from wcmatch import glob

        for file in glob.glob("*kfs*", flags=glob.EXTGLOB, root_dir=self.kfs_dir):
            shutil.copy(path.join(self.kfs_dir, file), self.kfc_dir)
        shutil.rmtree(self.kfs_dir)

        self.output.success("PyInstaller done")

    def __run_nuitka(self, build_type):
        with tools.chdir(path.pardir):
            self.__run_yarn(
                True,
                "nuitka",
                "--output-dir=build",
                path.join("src", "python", "kfc.py"),
            )

        kfc_dist_dir = path.join(self.build_dir, "kfc.dist")
        shutil.copytree(build_type, kfc_dist_dir)
        tools.rmdir(self.kfc_dir)
        shutil.move(kfc_dist_dir, self.kfc_dir)

        self.output.success("Nuitka done")

    def __run_freeze(self, build_type):
        os.environ["KFC_PYI_HOOKS_PATH"] = self.pyi_hooks_dir
        freeze = {"pyinstaller": self.__run_pyinstaller, "nuitka": self.__run_nuitka}
        freeze[str(self.options.freezer)](build_type)
