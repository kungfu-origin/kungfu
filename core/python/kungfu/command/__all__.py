# pyinstaller matters
# must explicitly import all commands

from . import master
from . import watcher
from . import gateway
from . import strategy
from . import ping
from . import unittest

from kungfu.command.extension import __all__
from kungfu.command.journal import __all__

from kungfu.command.dev import __all__
from kungfu.command.test import __all__
