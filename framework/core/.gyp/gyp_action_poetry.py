#  SPDX-License-Identifier: Apache-2.0

import os

from gyp_action_lib import Shell

opt = [] if "GITHUB_ACTIONS" in os.environ else ["-q"]

Shell.run(["yarn", "-s", "poetry", "lock"] + opt)
Shell.run(["yarn", "-s", "poetry", "install"] + opt)
Shell.touch("poetry.lock")
