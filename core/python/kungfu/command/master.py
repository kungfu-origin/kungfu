from kungfu.command import command, arg
from kungfu.practice.master import Master
from kungfu.practice.apprentice import Apprentice


@command(help='kungfu master', enable_daemonize=True)
def master(args, logger):
    server = Master(args, logger)
    server.go()


@command(help='kungfu apprentice test', enable_daemonize=True)
def app(args, logger):
    apprentice = Apprentice(args, logger)
    apprentice.go()
