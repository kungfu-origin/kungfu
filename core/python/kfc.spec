import os
import glob
from PyInstaller.utils.hooks import collect_data_files, collect_submodules

# hack from https://github.com/pyinstaller/pyinstaller/issues/4064
# to get pandas working
import distutils
if distutils.distutils_path.endswith('__init__.py'):
    distutils.distutils_path = os.path.dirname(distutils.distutils_path)

sep = os.path.sep
src_dir = os.path.abspath(os.path.join(os.path.curdir, 'python'))
cpp_dir = os.path.abspath(os.path.join(os.path.curdir, 'cpp'))
deps_dir = os.path.abspath(os.path.join(os.path.curdir, 'deps'))
build_dir = os.path.abspath(os.path.join(os.path.curdir, 'build'))
build_output_dir = os.path.join(build_dir, os.environ['CMAKE_BUILD_TYPE'])
ext_dir = os.path.join(src_dir, 'extensions')
ext_output_dir = os.path.join(build_dir, 'build_extensions')

print(f" !!! build_dir {build_dir}")
print(f" !!! build_output_dir {build_output_dir}")

datas = [
    (build_output_dir + os.path.sep + '*', '.'),
    (ext_dir, 'extensions'),
    (ext_output_dir, 'extensions'),
    (cpp_dir + sep + 'yijinjing' + sep + 'include', 'include'),
    (cpp_dir + sep + 'wingchun' + sep + 'include', 'include')
]

datas.extend(collect_data_files('tushare'))
datas.extend(collect_data_files('plotly'))

hiddenimports = [
    'numpy',
    'pandas',
    'tushare',
    'plotly',
    "recordclass",
    "sortedcontainers",
    "dotted_dict"
]

hiddenimports.extend(collect_submodules('pkg_resources'))
hiddenimports.extend(collect_submodules('numpy'))
hiddenimports.extend(collect_submodules('pandas'))
hiddenimports.extend(collect_submodules('plotly'))

block_cipher = None
a = Analysis(['kungfu\\__main__.py'],
     pathex=['python'],
     binaries=[],
     datas=[
        ('..\\build\\cpp\\yijinjing\\src\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('..\\build\\cpp\\wingchun\\src\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('..\\build\\deps\\fmt-5.3.0\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('..\\build\\deps\\SQLiteCpp-2.3.0\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('..\\build\\deps\\sqlite3-3.30.1\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('..\\build\\deps\\nanomsg-1.1.5\\src\\' + os.environ['CMAKE_BUILD_TYPE'] + '\\*.lib', '.'),
        ('extensions', 'extensions'),
        ('..\\build\\build_extensions', 'extensions'),
        ('..\\cpp\\yijinjing\\include', 'include'),
        ('..\\cpp\\wingchun\\include', 'include'),
        ('..\\build\\deps\\nanomsg-1.1.5\\include', 'include'),
        ('..\\deps\\spdlog-1.3.1\\include', 'include'),
        ('..\\deps\\json-3.5.0\\single_include', 'include'),
        ('..\\deps\\fmt-5.3.0\\include', 'include'),
        ('..\\deps\\rxcpp-4.1.0\\include', 'include'),
        ('..\\deps\\hffix-b67d404f\\include', 'include'),
        ('..\\deps\\SQLiteCpp-2.3.0\\include', 'include'),
        ('..\\deps\\pybind11-2.2.4', 'pybind11')
     ],
     hiddenimports=hiddenimports,
     hookspath=['python' + os.path.sep + 'hooks'],
     runtime_hooks=None,
     excludes=['extensions'],
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