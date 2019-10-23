
from PyInstaller.utils.hooks import collect_data_files, collect_dynamic_libs
hiddenimports = [
    "pyarrow._parquet",
    "pyarrow.formatting",
    "pyarrow.lib",
    "pyarrow.compat",
    "pyarrow.compat.*"
]

datas = collect_data_files('pyarrow')

binaries = collect_dynamic_libs('pyarrow')