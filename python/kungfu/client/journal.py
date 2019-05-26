import nnpy, pyyjj, pywingchun
from kungfu.command import command, arg

@arg("-k", "--keep", dest="keep", action = "store_true", help = "Keep listening")
@arg("-d", "--detail", dest = "need_detail", action='store_true', help = "Print Data Detail Or Not")
@arg("-e", "--end-time", dest="end_time", help = "End Time(%%Y%%m%%d-%%H:%%M:%%S)")
@arg("-s", "--start-time", dest= "start_time", help="Start Time (%%Y%%m%%d-%%H:%%M:%%S)")
@arg("-j", "--journal-name", dest = "jname", required = True, help = "Journal Name")
@arg("-f", "--folder", dest = "folder", help = "Journal Folder")
@command(help='journal tools')
def journal(args, logger):
    printer = pywingchun.JournalPrinter(args.folder, args.name, 20, False, False, True, args.need_detail)
    start_time = "20000101-13:30:00" if args.start_time is None else args.start_time
    end_time = "20200101-00:00:00" if args.end_time is None else args.end_time
    printer.run(start_time, end_time, args.keep)