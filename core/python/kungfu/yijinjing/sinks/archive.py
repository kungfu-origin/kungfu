import os
from kungfu.yijinjing.time import DAY_IN_NANO
from kungfu.yijinjing.locator import Locator
from pykungfu import yijinjing as yjj


class ArchiveSink(yjj.sink):
    def __init__(self, ctx):
        yjj.sink.__init__(self)
        self.ctx = ctx
        self.archive_locator = Locator(ctx.archive_dir)
        self.last_day = 0
        self.locator = None
        self.writer_maps = {}

    def put(self, location, dest_id, frame):
        date = int(frame.gen_time / DAY_IN_NANO)
        if date > self.last_day:
            date_str = yjj.strftime(date * DAY_IN_NANO, '%Y-%m-%d')
            self.ctx.logger.info(f'making archive for {date_str}')
            self.locator = Locator(os.path.join(self.ctx.archive_dir, date_str))
            self.writer_maps = {}
            self.last_day = date
        if location.uid not in self.writer_maps:
            self.writer_maps[location.uid] = {}
        writers = self.writer_maps[location.uid]
        if dest_id not in writers:
            target_location = yjj.location(location.mode, location.category, location.group, location.name, self.locator)
            writers[dest_id] = yjj.writer(target_location, dest_id, True, self.publisher)
        writers[dest_id].copy_frame(frame)
