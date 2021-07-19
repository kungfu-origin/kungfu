from kungfu.command import __all__ as commands
import kungfu.command as kfc

# Nuitka Settings
###########################################################
# nuitka-project: --standalone
# nuitka-project: --output-dir=../build
# nuitka-project: --remove-output
# nuitka-project: --assume-yes-for-downloads
# nuitka-project: --prefer-source-code
# nuitka-project: --static-libpython=yes
# nuitka-project: --python-flag=nosite
# nuitka-project: --python-flag=no_warnings
# nuitka-project: --warn-unusual-code
# nuitka-project: --warn-implicit-exceptions
# nuitka-project: --include-package=numpy
# nuitka-project: --include-package=pandas
# nuitka-project: --include-package=plotly
# nuitka-project: --include-package=wcwidth
# nuitka-project: --include-package=recordclass
# nuitka-project: --include-package=sortedcontainers
# nuitka-project: --include-package=dotted_dict
# nuitka-project: --include-package=yaml
# nuitka-project: --enable-plugin=numpy
###########################################################

# Environment Variables
###########################################################
# KF_HOME - base folder for kungfu files
# KF_LOG_LEVEL - logging level
# KF_NO_EXT - disable extensions if set
###########################################################


def main():
    kfc.execute()


if __name__ == "__main__":
    dir(commands)
    main()
