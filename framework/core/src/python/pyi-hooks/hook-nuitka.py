import PyInstaller
from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import logger

from os.path import (
    abspath,
    dirname,
    join as make_path,
)


site_path = abspath(dirname(dirname(PyInstaller.__file__)))

nuitka_dst_dir = make_path("nuitka")
nuitka_build_dst_dir = make_path("nuitka", "build")
nuitka_build_src_dir = make_path(site_path, "nuitka", "build")
data_nuitka_inline_copy = make_path(nuitka_build_src_dir, "inline_copy")

# https://pyinstaller.org/en/v5.1/hooks.html#hook-global-variables
datas = collect_data_files("nuitka", excludes=[])
datas.append((data_nuitka_inline_copy, make_path(nuitka_build_dst_dir, "inline_copy")))

logger.debug("datas: %s", datas)
