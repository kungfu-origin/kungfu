import kungfu
import sys

from pathlib import Path


def sdk(**kwargs):
    sdk_script = Path("../../developer/sdk/src/kfs.js").resolve().as_posix()
    sdk_args = [sys.executable, sdk_script, *sys.argv[1:]]
    kungfu.__binding__.libnode.run(*sdk_args, **kwargs)
