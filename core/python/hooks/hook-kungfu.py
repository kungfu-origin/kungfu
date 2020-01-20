import os
from PyInstaller.utils.hooks import collect_data_files, collect_submodules

build_dir = os.path.abspath(os.path.join(os.path.curdir, 'build'))
build_output_dir = os.path.join(build_dir, os.environ['CMAKE_BUILD_TYPE'])

print(f" !!! build_dir {build_dir}")
print(f" !!! build_output_dir {build_output_dir}")

datas = [
    (build_output_dir + os.path.sep + '*', '.')
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
