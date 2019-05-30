import kungfu.yijinjing.io as kfio
import kungfu.yijinjing.time as kft
import pyyjj


class JournalFinder:
    def __init__(self, args, logger):
        self._args = args
        self._logger = logger
        self._io_device = pyyjj.io_device(pyyjj.setup_log(args.name), False, False)
        kfio.checkin(args, logger, self._io_device)
        self._reader = self._io_device.open_reader_to_subscribe()
        self._reader.subscribe(pyyjj.mode.LIVE, pyyjj.category.MD, "xtp", "xtp", 0)
        self._sessions = self._reader.get_sessions()
        print('finding journal sessions:')
        for i in range(len(self._sessions)):
            s = self._sessions[i]
            session_meta = '[{:2d}] [{}] [{}]'.format(i + 1, s.fileinfo.category, s.fileinfo.group)
            session_info = '[{} - {}{}'.format(kft.strftime(s.start_time), kft.strftime(s.end_time), ']' if s.closed else ')')
            print('{} {}'.format(session_meta, session_info))

