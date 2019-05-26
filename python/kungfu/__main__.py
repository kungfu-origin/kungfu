# Environment variables that affects:
#   KF_HOME - base folder for kungfu files
#   KF_LOG_LEVEL - logging level
#   KF_NO_EXT - disable extensions if set

import kungfu.command as kfc
import kungfu.service
import kungfu.client
import kungfu.extension
import extensions
kfc.execute()