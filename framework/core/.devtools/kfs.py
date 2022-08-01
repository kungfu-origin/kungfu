from env import __frozen__
from kungfu import __tool__ as origin
from os import environ
from pathlib import Path

environ["KFS_PATH"] = Path("../../developer/sdk/dist/sdk/kfs.js").resolve().as_posix()

if __frozen__:
    origin.sdk()
