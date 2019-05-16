import argparse
import atexit
import os
import sys
import platform

def exit_handler(task):
    task.stop()

def create_subparser(subparsers, name, help):
    parser = subparsers.add_parser(name, usage='kfc ' + name + ' [<args>]', help=help)
    parser.set_defaults(command=name)
    parser.add_argument('-n', '--name', dest='name', help='Name for the process')
    return parser

class KungFu(object):

    def __init__(self):
        parser = argparse.ArgumentParser(description='Kung Fu Command', usage='kfc [<options>] <command> [<args>]')
        parser.add_argument('-V', '--version', action='store_true', help='Python version')
        subparsers = parser.add_subparsers(help='sub-command help')

        parser_paged = create_subparser(subparsers, 'paged', help = 'page server')
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

        # use dispatch pattern to invoke method with same name
        getattr(self, args.command)(args)

    def paged(self, args):
        from kungfu.mod import PageServer
        server = PageServer.PageServer()
        atexit.register(exit_handler, task=server)
        server.start()

    def calendar(self, args):
        from kungfu.mod import Calendar
        service = Calendar.Calendar(os.getenv('KF_HOME'))
        service.start()

    def md_sandbox(self, args):
        from kungfu.mod import MDSandbox
        md = MDSandbox.MDSandbox()
        atexit.register(exit_handler, task = md)
        md.run()

    def md_ctp(self, args):
        from kungfu.mod import MDCtp
        md = MDCtp.MDCtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_ctp(self, args):
        from kungfu.mod import TDCtp
        td = TDCtp.TDCtp(args.name)
        atexit.register(exit_handler, task = td)
        td.run()

    def md_xtp(self, args):
        from kungfu.mod import MDXtp
        md = MDXtp.MDXtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_xtp(self, args):
        from kungfu.mod import TDXtp
        td = TDXtp.TDXtp(args.name)
        atexit.register(exit_handler, task = td)
        td.run()

    def journal(self, args):
        import pyjournalprinter
        printer = pyjournalprinter.JournalPrinter(args.folder, args.name, 20, False, False, True, args.need_detail)
        start_time = "20000101-13:30:00" if args.start_time is None else args.start_time
        end_time = "20200101-00:00:00" if args.end_time is None else args.end_time
        printer.run(start_time, end_time, args.keep)

    def strategy(self, args):
        from kungfu.wingchun import strategy
        strategy_process = strategy.Strategy(args.name, args.path)
        strategy_process.run()

if __name__ == '__main__':
    KungFu()
