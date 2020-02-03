'''
Copyright [taurus.ai]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

import os
from .env import setup_environment_variables
from .msg import monkey_patch
from .version import get_version

setup_environment_variables()
monkey_patch()

version_file_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "version.info"))
if os.path.exists(version_file_path):
    with open(version_file_path, 'r') as version_file:
        __version__ = version_file.readline()
else:
    __version__ = get_version()
