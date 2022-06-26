import os
import PyInstaller

from os.path import (
    abspath,
    dirname,
    join as make_path,
)

site_path = abspath(dirname(dirname(PyInstaller.__file__)))

print(f'site_path: {site_path}')


def get_hook_dirs():
    return [os.path.dirname(__file__)]


def get_PyInstaller_tests():
    return [os.path.dirname(__file__)]
