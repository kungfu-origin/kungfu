#  SPDX-License-Identifier: Apache-2.0

import os


def get_hook_dirs():
    return [os.path.dirname(__file__)]


def get_PyInstaller_tests():
    return [os.path.dirname(__file__)]
