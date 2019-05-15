import argparse
import atexit
import os
import sys

def exit_handler(task):
    task.stop()

class KungFu(object):

    def __init__(self):
        parser = argparse.ArgumentParser(
            description='Kung Fu Command',
            usage='''kungfu <command> [<args>]

The supported kungfu commands are:
   paged    Start page engine
   client   Start page client
   calendar Start calendar service
   strategy Start strategy
''')
        parser.add_argument('command', help='Subcommand to run')
        parser.add_argument('-n', '--name', dest='name', help='Name for the process')
        parser.add_argument('-t', '--timeout', dest='timeout', type=int, default=1, help='Timeout in seconds')

        journal_group = parser.add_argument_group('journal options')
        journal_group.add_argument("--folder", "-f", dest = "folder", help = "Journal Folder")
        journal_group.add_argument("--journal-name", "-j", dest = "jname", help = "Journal Name")
        journal_group.add_argument("--detail", "-d", action='store_true', dest = "need_detail", help = "Print Data Detail Or Not")
        journal_group.add_argument("--start-time", "-s", dest= "start_time", help="Start Time (%%Y%%m%%d-%%H:%%M:%%S)")
        journal_group.add_argument("--end-time", "-e", dest="end_time", help = "End Time(%%Y%%m%%d-%%H:%%M:%%S)")
        journal_group.add_argument("--keep", "-k", action = "store_true", dest="keep", help = "Keep listening")

        strategy_group = parser.add_argument_group('strategy options')
        strategy_group.add_argument('-p', '--path', type=str, help='path of strategy py file')
        # parse_args defaults to [1:] for args, but you need to
        # exclude the rest of the args too, or validation will fail
        args = parser.parse_args(sys.argv[1:])
        if not hasattr(self, args.command):
            print('Unrecognized command')
            parser.print_help()
            exit(1)

        # use dispatch pattern to invoke method with same name
        if args.command == "journal":
            self.journal(folder = args.folder, name = args.jname, start_time = args.start_time, end_time = args.end_time, need_detail = args.need_detail, keep = args.keep)
        elif args.command == "strategy":
            self.strategy(args.name, args.timeout, args.path)
        else:
            getattr(self, args.command)(args.name, args.timeout)

    def paged(self, name, timeout):
        from kungfu.mod import PageServer
        server = PageServer.PageServer(name, timeout)
        atexit.register(exit_handler, task=server)
        server.start()

    def client(self, name, timeout):
        from kungfu.mod import PageClient
        client = PageClient.PageClient(name, timeout)
        client.run()

    def calendar(self, name, timeout):
        from kungfu.mod import Calendar
        service = Calendar.Calendar(os.environ["KF_HOME"])
        service.start()

    def md_sandbox(self, name, timeout):
        from kungfu.mod import MDSandbox
        md = MDSandbox.MDSandbox()
        atexit.register(exit_handler, task = md)
        md.run()

    def md_ctp(self, name, timeout):
        from kungfu.mod import MDCtp
        md = MDCtp.MDCtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_ctp(self, name, timeout):
        from kungfu.mod import TDCtp
        td = TDCtp.TDCtp(name)
        atexit.register(exit_handler, task = td)
        td.run()

    def md_xtp(self, name, timeout):
        from kungfu.mod import MDXtp
        md = MDXtp.MDXtp()
        atexit.register(exit_handler, task = md)
        md.run()

    def td_xtp(self, name, timeout):
        from kungfu.mod import TDXtp
        td = TDXtp.TDXtp(name)
        atexit.register(exit_handler, task = td)
        td.run()

    def strategy(self, name, timeout, path):
        from kungfu.wingchun import strategy
        strategy_process = strategy.Strategy(name, path)
        strategy_process.run()

    def journal(self, folder, name, start_time, end_time, need_detail, keep):
        import pyjournalprinter
        printer = pyjournalprinter.JournalPrinter(folder, name, 20, False, False, True, need_detail)
        start_time = "20000101-13:30:00" if start_time is None else start_time
        end_time = "20200101-00:00:00" if end_time is None else end_time
        printer.run(start_time, end_time, keep)

    def run(self, name, timeout):
        sys.path.append('/user_strategy')
        exec(open(name).read())

if __name__ == '__main__':
    KungFu()
