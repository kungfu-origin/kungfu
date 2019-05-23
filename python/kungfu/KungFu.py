import argparse
import atexit
import os
import sys
import platform

from kungfu.log.Logger import create_logger

def exit_handler(task):
    task.stop()

def create_subparser(subparsers, name, help):
    parser = subparsers.add_parser(name, usage='kfc ' + name + ' [<args>]', help=help)
    parser.set_defaults(command=name)
    parser.add_argument('-n', '--name', dest='name', help='name for the process')
    parser.add_argument('-l', '--log-level', dest='log_level', type=str, choices=['trace','debug','info','warning','error','critical'], help='logging level')
    return parser

class KungFu(object):

    def __init__(self):
        parser = argparse.ArgumentParser(description='Kung Fu Command', usage='kfc [<options>] <command> [<args>]')
        parser.add_argument('-V', '--version', action='store_true', help='Python version')
        subparsers = parser.add_subparsers(help='sub-command help')

        parser_master = create_subparser(subparsers, 'master', help = 'kungfu master')
        parser_ping = create_subparser(subparsers, 'ping', help = 'ping kungfu master')
        parser_ping.add_argument('-m', '--message', dest='message', default='{}', type=str, help='message')
        parser_ping.add_argument('-t', '--times', dest='times', type=int, default=4, help='times')

        parser_calendar = create_subparser(subparsers, 'calendar', help = 'calendar service')
        parser_md_sandbox = create_subparser(subparsers, 'md_sandbox', help = 'Market Data from sandbox')
        parser_md_ctp = create_subparser(subparsers, 'md_ctp', help = 'Market Data from CTP')
        parser_td_ctp = create_subparser(subparsers, 'td_ctp', help = 'Trade Service for CTP')
        parser_md_xtp = create_subparser(subparsers, 'md_xtp', help = 'Market Data from XTP')
        parser_td_xtp = create_subparser(subparsers, 'td_xtp', help = 'Trade Service for XTP')
        
        parser_journal = create_subparser(subparsers, 'journal', help = 'journal tools')
        parser_journal.add_argument("-f", "--folder", dest = "folder", help = "Journal Folder")
        parser_journal.add_argument("-j", "--journal-name", dest = "jname", required = True, help = "Journal Name")
        parser_journal.add_argument("-d", "--detail", action='store_true', dest = "need_detail", help = "Print Data Detail Or Not")
        parser_journal.add_argument("-s", "--start-time", dest= "start_time", help="Start Time (%%Y%%m%%d-%%H:%%M:%%S)")
        parser_journal.add_argument("-e", "--end-time", dest="end_time", help = "End Time(%%Y%%m%%d-%%H:%%M:%%S)")
        parser_journal.add_argument("-k", "--keep", action = "store_true", dest="keep", help = "Keep listening")
        
        parser_strategy = create_subparser(subparsers, 'strategy', help = 'run strategy')
        parser_strategy.add_argument('-p', '--path', type=str, help='path of strategy py file')

        args = parser.parse_args()
        if args.version:
            print('Python', platform.python_version())
            sys.exit(0)
        if 'command' not in args:
            print('Unspecified command')
            parser.print_help()
            sys.exit(1)
        if not hasattr(self, args.command):
            print('Unrecognized command')
            parser.print_help()
            sys.exit(1)

        env_log_level = os.getenv('KF_LOG_LEVEL')
        if not env_log_level and not args.log_level:
            args.log_level = 'warning'
        if env_log_level and not args.log_level:
            args.log_level = os.environ['KF_LOG_LEVEL']
        os.environ['KF_LOG_LEVEL'] = args.log_level
        logger = create_logger(args.name if args.name else args.command, args.log_level)
        # use dispatch pattern to invoke method with same name
        getattr(self, args.command)(args, logger)

    def master(self, args, logger):
        from kungfu.service.server import Master
        server = Master(logger)
        atexit.register(exit_handler, task=server)
        server.start()

    def ping(self, args, logger):
        from kungfu.client.ping import Ping
        ping = Ping(logger)
        atexit.register(exit_handler, task=ping)
        ping.ping(args.message, args.times)

    def calendar(self, args, logger):
        from kungfu.mod import Calendar
        service = Calendar.Calendar(os.getenv('KF_HOME'))
        service.start()

    def md_sandbox(self, args, logger):
        from kungfu.mod import MDSandbox
        md = MDSandbox.MDSandbox()
        atexit.register(exit_handler, task = md)
        md.run()

    def md_ctp(self, args, logger):
        from kungfu.mod import MDCtp
        md = MDCtp.MDCtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_ctp(self, args, logger):
        from kungfu.mod import TDCtp
        td = TDCtp.TDCtp(args.name)
        atexit.register(exit_handler, task = td)
        td.run()

    def md_xtp(self, args, logger):
        from kungfu.mod import MDXtp
        md = MDXtp.MDXtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_xtp(self, args, logger):
        from kungfu.mod import TDXtp
        td = TDXtp.TDXtp(args.name)
        atexit.register(exit_handler, task = td)
        td.run()

    def journal(self, args, logger):
        import pyjournalprinter
        printer = pyjournalprinter.JournalPrinter(args.folder, args.name, 20, False, False, True, args.need_detail)
        start_time = "20000101-13:30:00" if args.start_time is None else args.start_time
        end_time = "20200101-00:00:00" if args.end_time is None else args.end_time
        printer.run(start_time, end_time, args.keep)

    def strategy(self, args, logger):
        from kungfu.client import strategy
        strategy_process = strategy.Strategy(logger, args.name, args.path)
        atexit.register(exit_handler, task = strategy_process)
        strategy_process.run()

if __name__ == '__main__':
    KungFu()
