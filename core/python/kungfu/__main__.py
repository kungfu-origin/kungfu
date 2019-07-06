# Environment variables that affects:
#   KF_HOME - base folder for kungfu files
#   KF_LOG_LEVEL - logging level
#   KF_NO_EXT - disable extensions if set


import extensions
import kungfu.command as kfc
from kungfu.command import __all__
kfc.execute()
