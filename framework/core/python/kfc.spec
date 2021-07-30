# PyInstaller Settings
###########################################################
import blib2to3
import glob
import nuitka
import os

from os import path
from os.path import abspath
from os.path import dirname
from os.path import curdir as cwd

from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_submodules

cmake_dir = abspath(path.join(cwd, ".cmake"))

deps_dir = abspath(path.join(cwd, ".deps"))
dep_pybind11_dir = abspath(path.join(deps_dir, "pybind11*"))

src_cpp_dir = abspath(path.join(cwd, "cpp"))
src_python_dir = abspath(path.join(cwd, "python"))
src_ext_dir = path.join(src_python_dir, "kungfu_extensions")

build_dir = abspath(path.join(cwd, "build"))
build_cpp_dir = abspath(path.join(build_dir, "cpp"))
build_deps_dir = abspath(path.join(build_dir, "deps"))
build_output_dir = path.join(build_dir, os.environ["CMAKE_BUILD_TYPE"])

data_blib2to3_dir = path.join(dirname(blib2to3.__file__))
data_nuitka_build_dir = path.join(dirname(nuitka.__file__), "build")


def extend_datas(datas, src_dirs, build_dirs, packages):
    def add_include(datas, path):
        map(
            lambda include: datas.append((include, "include")),
            glob.glob(path.join(path, "**", "include"), recursive=True),
        )

    def add_lib(datas, path):
        map(
            lambda lib: datas.append((lib, ".")),
            glob.glob(path.join(path, "**", "*.lib"), recursive=True),
        )

    map(add_include, src_dirs)
    map(add_lib, build_dirs)
    map(lambda pkg: datas.extend(collect_data_files(pkg)), packages)
    return datas


def extend_hiddenimports(hiddenimports):
    def is_valid(submodule):
        return submodule not in hiddenimports and "test" not in submodule

    for pkg_name in list(hiddenimports):
        hiddenimports.extend(filter(is_valid, collect_submodules(pkg_name)))
    return hiddenimports


block_cipher = None
a = Analysis(
    [path.join("kungfu", "__main__.py")],
    pathex=["python"],
    binaries=[],
    datas=extend_datas(
        [
            (cmake_dir, "cmake"),
            (src_ext_dir, "kungfu_extensions"),
            (dep_pybind11_dir, "pybind11"),
            (path.join(build_output_dir, "*"), "."),
            (path.join(build_dir, "include"), "include"),
            (path.join(data_blib2to3_dir, "*.txt"), "blib2to3"),
            (
                path.join(data_nuitka_build_dir, "include"),
                path.join("nuitka", "build", "include"),
            ),
            (
                path.join(data_nuitka_build_dir, "static_src"),
                path.join("nuitka", "build", "static_src"),
            ),
            (path.join(data_nuitka_build_dir, "*.scons"), path.join("nuitka", "build")),
        ],
        src_dirs=[
            src_cpp_dir,
            deps_dir,
            build_deps_dir,
        ],
        build_dirs=[
            build_cpp_dir,
            build_deps_dir,
        ],
        packages=[
            "plotly",
        ],
    ),
    hiddenimports=extend_hiddenimports(
        [
            "pkg_resources",
            "nuitka",
            "SCons",
            "numpy",
            "pandas",
            "plotly",
            "dotted_dict",
            "recordclass",
            "sortedcontainers",
            "wcwidth",
        ]
    ),
    excludes=[
        "kungfu_extensions",
        "matplotlib",
    ],
    hookspath=[],
    runtime_hooks=None,
    cipher=block_cipher,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)
exe = EXE(
    pyz,
    a.scripts,
    name="kfc",
    console=True,
    debug=False,
    exclude_binaries=True,
    strip=False,
)
coll = COLLECT(exe, a.binaries, a.zipfiles, a.datas, name="kfc", strip=False)
