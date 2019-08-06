import pyyjj
import pywingchun
from pywingchun.constants import MsgType

import kungfu.yijinjing.msg as kfm
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
from kungfu.yijinjing.log import create_logger

import sys

class Converter:

    def __init__(self, ctx):
        self.ctx = ctx
        self.ctx.converter = self
        self.dest_id = 0 # dest_id 0 should be configurable TODO
        self.location = pyyjj.location(kfj.MODES['backtest'], kfj.CATEGORIES['md'], ctx.group, ctx.name, ctx.locator)
        self.io_device = pyyjj.io_device(self.location, False, True)
        self.writer = self.io_device.open_writer(self.dest_id)
        self.reader = self.io_device.open_reader_to_subscribe()
        self.reader.join(self.location, self.dest_id, 0)
        self.logger = create_logger("converter", ctx.log_level, self.location)

    def mark_SessionStart(self, time):
        # session start time should be smaller than the first quote and greater than 0
        self.writer.mark_with_time(time, kfm.SessionStart)

    def mark_SessionEnd(self, time):
        # session end time should be equal or greater than the last quote
        self.writer.mark_with_time(time, kfm.SessionEnd)

    def write_data_to_journal(self):
        raise NotImplementedError

    def read_data_from_journal(self):
        frame_count = 0
        reader = self.reader 

        while reader.data_available():
            frame = reader.current_frame()
            frame_count = frame_count + 1
            print('*'*10 + ' frame-' + str(frame_count) + ' ' + '*'*10)
            print(frame.msg_type)
            if frame.msg_type == MsgType.Quote:
                quote = pywingchun.utils.get_quote(frame)
                print(quote)
            print("gen_time: {}".format(kft.strftime(frame.gen_time)))
            print("gen_time: {}".format(frame.gen_time))
            reader.next()
        self.logger.info("read journal success!")