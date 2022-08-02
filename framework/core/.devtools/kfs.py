from env import __frozen__, kfc_dir
from kungfu import __tool__ as origin
from os import environ
from pathlib import Path

environ["KFS_PATH"] = Path("../../developer/sdk/dist/sdk/kfs.js").resolve().as_posix()
environ["KFC_PATH"] = kfc_dir.replace('\\', '/')

if __frozen__:
    origin.sdk()
