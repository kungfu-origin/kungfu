#  SPDX-License-Identifier: Apache-2.0

import json
import os

import pykungfu as __binding__

with open(
    os.path.join(os.path.dirname(__binding__.__file__), "kungfubuildinfo.json"),
    "r",
) as build_info_file:
    __build_info__ = json.load(build_info_file)

__version__ = __build_info__["version"]
