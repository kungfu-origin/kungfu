# pyinstaller matters
# must explicitly import all commands

from . import master
from . import watcher
from . import gateway
from . import strategy
from . import ping

from kungfu.command.extension import __all__
from kungfu.command.journal import __all__
