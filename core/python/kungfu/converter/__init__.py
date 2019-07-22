import pyyjj
import pywingchun
from pywingchun.constants import MsgType

import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
from kungfu.log import create_logger

import pandas as pd
import sys
#from ctypes import c_char_p

class Converter:

    def __init__(self, ctx):
        self.ctx = ctx
        self.ctx.converter = self
        self.filepath = ctx.filepath
        self.from_time = int(ctx.time)
        self.data = None
        self.dext_id = 0

        self.location = pyyjj.location(kfj.MODES['backtest'], kfj.CATEGORIES['md'], ctx.grp, ctx.nm, ctx.locator)
        self.io_device = pyyjj.io_device(self.location, False, True)
        self.writer = self.io_device.open_writer(self.dext_id)
        self.reader = self.io_device.open_reader_to_subscribe()
        self.reader.join(self.location, self.dext_id, self.from_time)

        self.logger = create_logger("converter", ctx.log_level, self.location)

        self.logger.info("converter creation success!")

    def read_data_source(self):
        #transfer csv to df
        return pd.read_csv(open(self.filepath))

    def write_data_to_journal(self):
        df = self.read_data_source()

        for i in range(len(df)):
            tick_data = df.loc[i,:]

            frame_data = pywingchun.Quote()
            frame_data.data_time = tick_data.Timestamp
            frame_data.last_price = tick_data.LastPrice

            frame_data.__write__(tick_data.Timestamp, self.writer)
            break

        self.logger.info("write journal success!")

    def read_data_from_journal(self):
        frame_count = 0
        reader = self.reader

        while reader.data_available():
            frame = reader.current_frame()
            frame_count = frame_count + 1

            quote = pywingchun.utils.get_quote(frame)
            print('*'*10 + ' frame-' + str(frame_count) + ' ' + '*'*10)
            print("gen_time: {}\nlast_price: {}".format(kft.strftime(frame.gen_time), quote.last_price))
            reader.next()
        
        self.logger.info("read journal success!")