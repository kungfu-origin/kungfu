# Environment Variables
###########################################################
# KF_HOME - base folder for kungfu files
# KF_LOG_LEVEL - logging level
# KF_NO_EXT - disable extensions if set
###########################################################

from kungfu.console import commands, select, variants


def run(**kwargs):
    select(map(lambda mod: mod.pick, [variants, commands]), **kwargs)


if __name__ == "__main__":
    run()
