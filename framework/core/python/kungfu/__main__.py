# Environment Variables
###########################################################
# KF_HOME - base folder for kungfu files
# KF_LOG_LEVEL - logging level
# KF_NO_EXT - disable extensions if set
###########################################################

from kungfu.command import __all__ as commands
import kungfu.command as kfc


def run():
    id(commands)
    kfc.execute()


if __name__ == "__main__":
    run()
