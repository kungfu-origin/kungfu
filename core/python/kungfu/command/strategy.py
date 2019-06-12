from kungfu.command import command, arg
from kungfu.wingchun.strategy import Strategy
from kungfu.practice.apprentice import Apprentice, EventHandler


@arg('-p', '--path', type=str, help='path of strategy py file')
@command(help='run trading strategy')
def strategy(args, logger):
    handler = EventHandler(args, logger, Strategy(logger, args.name, args.path))
    apprentice = Apprentice(args, logger)
    apprentice.add_event_handler(handler)
    apprentice.go()
