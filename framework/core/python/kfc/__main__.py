# Nuitka Settings
###########################################################
# nuitka-project: --standalone
# nuitka-project: --output-dir=../build
# nuitka-project: --remove-output
# nuitka-project: --assume-yes-for-downloads
# nuitka-project: --prefer-source-code
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
# nuitka-project: --nofollow-import-to=pandas.tests
# nuitka-project: --nofollow-import-to=numpy.distutils
# nuitka-project: --enable-plugin=numpy
# nuitka-project: --enable-plugin=pylint-warnings
###########################################################

from kungfu import __main__ as origin


if __name__ == "__main__":
    origin.main()
