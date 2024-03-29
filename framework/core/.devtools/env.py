#  SPDX-License-Identifier: Apache-2.0

import os
import sys

from os.path import abspath, dirname

base_dir = dirname(abspath(dirname(__file__)))
python_dir = os.path.join(base_dir, "src", "python")
kfc_dir = os.path.join(base_dir, "dist", "kfc")
build_info_file = os.path.join(kfc_dir, "kungfubuildinfo.json")

sys.path.append(python_dir)
sys.path.append(kfc_dir)
os.environ["PATH"] += os.pathsep + kfc_dir
os.environ["KF_LOG_LEVEL"] = "trace"


__frozen__ = os.path.exists(kfc_dir) and os.path.exists(build_info_file)
