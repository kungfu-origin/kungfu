# Nuitka Settings
###########################################################
# nuitka-project: --standalone
#
# nuitka-project: --assume-yes-for-downloads
# nuitka-project: --prefer-source-code
# nuitka-project: --python-flag=nosite
# nuitka-project: --python-flag=no_warnings
# nuitka-project: --remove-output
# nuitka-project: --static-libpython=no
#
# nuitka-project: --warn-unusual-code
# nuitka-project: --warn-implicit-exceptions
#
# nuitka-project: --include-package=numpy
# nuitka-project: --include-package=pandas
# nuitka-project: --include-package=plotly
# nuitka-project: --include-package=wcwidth
# nuitka-project: --include-package=recordclass
# nuitka-project: --include-package=sortedcontainers
# nuitka-project: --include-package=dotted_dict
# nuitka-project: --include-package=yaml
#
# nuitka-project: --nofollow-import-to=*.distutils
# nuitka-project: --nofollow-import-to=*.tests
#
# nuitka-project: --enable-plugin=anti-bloat
# nuitka-project: --enable-plugin=numpy
# nuitka-project: --enable-plugin=pylint-warnings
#
# nuitka-project: --noinclude-pytest-mode=nofollow
# nuitka-project: --noinclude-setuptools-mode=nofollow
# nuitka-project: --noinclude-custom-mode=distutils:nofollow
#
# nuitka-project: --no-progress
###########################################################

from kungfu import __main__ as origin


if __name__ == "__main__":
    origin.main()