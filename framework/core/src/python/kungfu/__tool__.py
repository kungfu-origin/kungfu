import kungfu
import sys

from pathlib import Path


def sdk(**kwargs):
    sdk_script = ''
    if (process.env.KFS_DEV_PATH) {
        sdk_script = process.env.KFS_DEV_PATH
    } else {
        Path(sys.executable), ../.
        sdk_script = Path("../../../app/dist/kfs/kfs.js").resolve().as_posix()
    }

    sdk_args = [sys.executable, sdk_script, *sys.argv[1:]]
    kungfu.__binding__.libnode.run(*sdk_args, **kwargs)
