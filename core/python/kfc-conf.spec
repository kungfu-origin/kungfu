import os
import glob
from PyInstaller.utils.hooks import collect_data_files, collect_submodules


sep = os.path.sep
src_dir = os.path.abspath(os.path.join(os.path.curdir, 'python'))
cmake_dir = os.path.abspath(os.path.join(os.path.curdir, 'cmake'))
cpp_dir = os.path.abspath(os.path.join(os.path.curdir, 'cpp'))
deps_dir = os.path.abspath(os.path.join(os.path.curdir, 'deps'))
pybind_dir = os.path.abspath(os.path.join(deps_dir, 'pybind11*'))
build_dir = os.path.abspath(os.path.join(os.path.curdir, 'build'))
build_cpp_dir = os.path.abspath(os.path.join(build_dir, 'cpp'))
build_deps_dir = os.path.abspath(os.path.join(build_dir, 'deps'))
build_output_dir = os.path.join(build_dir, os.environ['CMAKE_BUILD_TYPE'])
ext_dir = os.path.join(src_dir, 'kungfu_extensions')
ext_output_dir = os.path.join(build_dir, 'build_extensions')

datas = [
    (cmake_dir, 'cmake'),
    (build_output_dir + sep + '*', '.'),
    (ext_dir, 'kungfu_extensions'),
    (ext_output_dir, 'kungfu_extensions'),
    (pybind_dir, 'pybind11'),
    (build_dir + sep + 'include', 'include')
]

def add_include(path):
    for include in glob.glob(path + sep + '**' + sep + 'include', recursive=True):
        datas.append((include, 'include'))

add_include(cpp_dir)
add_include(deps_dir)
add_include(build_dir + sep + 'deps')

def add_lib(path):
    for lib in glob.glob(path + sep + '**' + sep + '*.lib', recursive=True):
        datas.append((lib, '.'))

add_lib(build_cpp_dir)
add_lib(build_deps_dir)

datas.extend(collect_data_files('tushare'))
datas.extend(collect_data_files('plotly'))
datas.extend(collect_data_files('wcwidth'))

hiddenimports = [
    'kungfu.data.adapter',
    'kungfu.yijinjing.sinks.csv',
    'numpy',
    'pandas',
    'rqalpha',
    'tushare',
    'plotly',
    'wcwidth',
    "recordclass",
    "sortedcontainers",
    "dotted_dict",
    "yaml"
]

def extend_hiddenimports(pkg_name):
    hiddenimports.extend(filter(lambda e: 'test' not in e, collect_submodules(pkg_name)))

extend_hiddenimports('pkg_resources')
extend_hiddenimports('numpy')
extend_hiddenimports('pandas')
extend_hiddenimports('plotly')

block_cipher = None
a = Analysis(['kungfu' + os.path.sep + '__main__.py'],
     pathex=['python'],
     binaries=[],
     datas=datas,
     hiddenimports=hiddenimports,
     hookspath=[],
     runtime_hooks=None,
     excludes=['kungfu_extensions', 'matplotlib'],
     cipher=block_cipher)
pyz = PYZ(a.pure, a.zipped_data,
     cipher=block_cipher)
exe = EXE(pyz,
                a.scripts,
                exclude_binaries=True,
                name='kfc',
                debug=False,
                strip=False,
                upx=True,
                console=True )
coll = COLLECT(exe,
                    a.binaries,
                    a.zipfiles,
                    a.datas,
                    strip=False,
                    upx=True,
                    name='kfc')