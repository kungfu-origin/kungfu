"""
Copyright [www.kungfu-trader.com]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import json
import os
import pykungfu
import sys

with open(
    os.path.join(os.path.dirname(pykungfu.__file__), "kungfubuildinfo.json"),
    "r",
) as build_info_file:
    __build_info__ = json.load(build_info_file)

__version__ = __build_info__["version"]
__bindings__ = pykungfu


def export(fn):
    mod = sys.modules[fn.__module__]
    if hasattr(mod, "__all__"):
        mod.__all__.append(fn.__name__)
    else:
        mod.__all__ = [fn.__name__]
    return fn
