#  SPDX-License-Identifier: Apache-2.0

import kungfu
import sys
import os

from pathlib import Path


def sdk(**kwargs):
    prod_sdk_script_path = str(
        (Path(sys.executable) / "../../app/dist/kfs/kfs.js").resolve()
    )
    sdk_script_path = os.getenv("KFS_PATH", prod_sdk_script_path)
    sdk_script = Path(sdk_script_path).resolve().as_posix()
    sdk_args = [sys.executable, sdk_script, *sys.argv[1:]]
    kungfu.__binding__.libnode.run(*sdk_args, **kwargs)
