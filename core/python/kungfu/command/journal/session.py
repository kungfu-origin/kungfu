import kungfu.command.journal as kfj
from kungfu.yijinjing.journal import JournalFinder
from kungfu.yijinjing.time import *
import pyyjj


@kfj.arg('-g', '--group', dest='group', type=str, help='data group')
@kfj.arg('-c', '--category', dest='category', type=str, help='data category')
@kfj.command(help='list sessions')
def session(args, logger):
    finder = JournalFinder(args, logger)
