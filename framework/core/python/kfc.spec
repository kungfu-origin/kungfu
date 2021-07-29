# PyInstaller Settings
###########################################################
import glob
import os

from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_submodules

src_dir = os.path.abspath(os.path.join(os.path.curdir, "python"))
cmake_dir = os.path.abspath(os.path.join(os.path.curdir, ".cmake"))
deps_dir = os.path.abspath(os.path.join(os.path.curdir, ".deps"))
cpp_dir = os.path.abspath(os.path.join(os.path.curdir, "cpp"))
pybind_dir = os.path.abspath(os.path.join(deps_dir, "pybind11*"))
build_dir = os.path.abspath(os.path.join(os.path.curdir, "build"))
build_cpp_dir = os.path.abspath(os.path.join(build_dir, "cpp"))
build_deps_dir = os.path.abspath(os.path.join(build_dir, "deps"))
build_output_dir = os.path.join(build_dir, os.environ["CMAKE_BUILD_TYPE"])
ext_dir = os.path.join(src_dir, "kungfu_extensions")


def extend_datas(datas, src_dirs, build_dirs, packages):
    def add_include(datas, path):
        map(
            lambda include: datas.append((include, "include")),
            glob.glob(os.path.join(path, "**", "include"), recursive=True),
        )

    def add_lib(datas, path):
        map(
            lambda lib: datas.append((lib, ".")),
            glob.glob(os.path.join(path, "**", "*.lib"), recursive=True),
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
    [os.path.join("kungfu", "__main__.py")],
    pathex=["python"],
    binaries=[],
    datas=extend_datas(
        [
            (cmake_dir, "cmake"),
            (ext_dir, "kungfu_extensions"),
            (pybind_dir, "pybind11"),
            (os.path.join(build_output_dir, "*"), "."),
            (os.path.join(build_dir, "include"), "include"),
        ],
        src_dirs=[
            cpp_dir,
            deps_dir,
            build_deps_dir,
        ],
        build_dirs=[
            build_cpp_dir,
            build_deps_dir,
        ],
        packages=[
            "nuitka",
            "plotly",
        ],
    ),
    hiddenimports=extend_hiddenimports(
        [
            "nuitka.build",
            "nuitka.plugins.standard",
            "SCons",
            "SCons.Platform",
            "SCons.Tool",
            "pkg_resources",
            "numpy",
            "pandas",
            "plotly",
            "wcwidth",
            "recordclass",
            "sortedcontainers",
            "dotted_dict",
            "yaml",
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
