#  SPDX-License-Identifier: Apache-2.0

from PyInstaller.utils.hooks import collect_data_files

# https://pyinstaller.org/en/v5.1/hooks.html#hook-global-variables
datas = collect_data_files("blib2to3")
