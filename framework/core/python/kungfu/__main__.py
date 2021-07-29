# Environment Variables
###########################################################
# KF_HOME - base folder for kungfu files
# KF_LOG_LEVEL - logging level
# KF_NO_EXT - disable extensions if set
###########################################################

from kungfu.commands import __list__ as commands
from kungfu.commands import __run__ as run


if __name__ == "__main__":
    id(commands)
    run()
