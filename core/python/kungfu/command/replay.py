from kungfu.command import command, arg
from kungfu.wingchun.strategy import Strategy
from kungfu.practice.apprentice import Apprentice, EventHandler

@arg('-p', '--path', type=str, help='path of strategy py file')
@command(help='replay trading strategy')
def replay(args, logger):
    logger.info('strategy command try')
    strategy = Strategy(logger, args.name, args.path)
    handler = EventHandler(logger, strategy)
    apprentice = Apprentice(args, logger)
    apprentice.add_event_handler(handler)
    apprentice.go()