# PyInstaller Settings
###############################################################################
import glob
import platform
import os
import PyInstaller

from collections import deque
from distutils import sysconfig
from os.path import (
    abspath,
    basename,
    dirname,
    curdir as cwd,
    join as make_path,
)
from PyInstaller.building.api import COLLECT, EXE, PYZ
from PyInstaller.building.build_main import Analysis
from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_submodules

###############################################################################
# python dir
python_inc = sysconfig.get_python_inc(plat_specific=True)
python_dir = (
    dirname(python_inc)
    if basename(python_inc) == "include"
    else dirname(dirname(python_inc))
)

# cmake includes
cmake_dir = abspath(make_path(cwd, ".cmake"))

# cpp dependencies
deps_dir = abspath(make_path(cwd, ".deps"))
dep_pybind11_dir = abspath(make_path(deps_dir, "pybind11*"))

# kungfu source files
src_dir = abspath(make_path(cwd, "src"))

# kungfu build files
build_dir = abspath(make_path(cwd, "build"))
build_cpp_dir = abspath(make_path(build_dir, "src"))
build_output_dir = make_path(build_dir, os.environ["CMAKE_BUILD_TYPE"])

# site path
site_path = abspath(dirname(dirname(PyInstaller.__file__)))

# black requires
data_blib2to3 = make_path(site_path, "blib2to3", "*.txt")

# pdm requires
data_pep517 = make_path(site_path, "pep517", "in_process", "_in_process.py")
data_pdm_models = make_path(site_path, "pdm", "models", "*.json")
data_pdm_pep582 = make_path(site_path, "pdm", "pep582", "sitecustomize.py")
data_pip_certifi = make_path(site_path, "pip", "_vendor", "certifi", "*.pem")

###############################################################################


def extend_datas(datas, src_dirs, build_dirs, packages):
    def add_include(path):
        deque(
            map(
                lambda include: datas.append((include, "include")),
                glob.glob(make_path(path, "**", "include"), recursive=True),
            )
        )

    def add_lib(path):
        deque(
            map(
                lambda lib: datas.append((lib, ".")),
                glob.glob(make_path(path, "**", "*.lib"), recursive=True),
            )
        )

    deque(map(add_include, src_dirs))
    deque(map(add_lib, build_dirs))
    deque(map(lambda pkg: datas.extend(collect_data_files(pkg)), packages))

    if platform.system() == "Windows":
        datas.append((make_path(python_dir, "libs"), "libs"))
    return datas


def extend_hiddenimports(modules, executable_modules):
    hiddenimports = list(modules)

    def is_valid(submodule):
        return submodule not in hiddenimports and "test" not in submodule

    for pkg_name in modules:
        hiddenimports.extend(filter(is_valid, collect_submodules(pkg_name)))
    for pkg_name in executable_modules:
        hiddenimports.append(f"{pkg_name}.__main__")

    return hiddenimports


def get_hookspath():
    key = "KFC_PYI_HOOKS_PATH"
    return [] if key not in os.environ else os.environ[key].split(os.pathsep)


def get_runtimehooks():
    key = "KFC_PYI_RUNTIME_HOOKS"
    return None if key not in os.environ else os.environ[key].split(",")


###############################################################################
name = "kfc"
block_cipher = None
a = Analysis(
    scripts=["kfc.py"],
    pathex=[],
    binaries=[],
    datas=extend_datas(
        [
            (cmake_dir, "cmake"),
            (dep_pybind11_dir, "pybind11"),
            (make_path(build_output_dir, "*"), "."),
            (make_path(build_dir, "include"), "include"),
            (data_blib2to3, "blib2to3"),
            (data_pep517, make_path("pep517", "in_process")),
            (data_pdm_models, make_path("pdm", "models")),
            (data_pdm_pep582, make_path("pdm", "pep582")),
            (data_pip_certifi, make_path("pip", "_vendor", "certifi")),
        ],
        src_dirs=[
            src_dir,
        ],
        build_dirs=[
            build_cpp_dir,
        ],
        packages=[],
    ),
    hiddenimports=extend_hiddenimports(
        modules=[
            "black",
            "pip._internal",
            "pip._vendor",
            "pkg_resources",
            "pdm",
            "pep517",
            "shellingham",
            "nuitka",
            "ordered_set",
            "SCons",
            "setuptools",
            "numpy",
            "pandas",
        ],
        executable_modules=[
            "kungfu",
            "pip",
        ],
    ),
    excludes=[
        "matplotlib",
    ],
    hookspath=get_hookspath(),
    runtime_hooks=get_runtimehooks(),
    cipher=block_cipher,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)
exe = EXE(
    pyz,
    a.scripts,
    name=name,
    console=True,
    debug=False,
    exclude_binaries=True,
    strip=False,
)
coll = COLLECT(exe, a.binaries, a.zipfiles, a.datas, name=name, strip=False)
