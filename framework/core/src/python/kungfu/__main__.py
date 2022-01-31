# Environment Variables
###########################################################
# KF_HOME - base folder for kungfu files
# KF_LOG_LEVEL - logging level
# KF_NO_EXT - disable extensions if set
###########################################################

from kungfu.console import available, select


def main(**kwargs):
    select(available(), **kwargs)


if __name__ == "__main__":
    main()
