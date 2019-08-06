import pyyjj
import pywingchun
from pywingchun.constants import MsgType

import kungfu.yijinjing.msg as kfm
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
from kungfu.yijinjing.log import create_logger

import pandas as pd
import sys

class Converter:

    def __init__(self, ctx):
        self.ctx = ctx
        self.ctx.converter = self
        self.filepath = ctx.filepath
        self.from_time = int(ctx.time)
        self.data = None
        # dest_id 0 should be configurable TODO
        self.dest_id = 0

        self.location = pyyjj.location(kfj.MODES['backtest'], kfj.CATEGORIES['md'], ctx.group, ctx.name, ctx.locator)
        self.io_device = pyyjj.io_device(self.location, False, True)
        self.writer = self.io_device.open_writer(self.dest_id)
        self.reader = self.io_device.open_reader_to_subscribe()
        self.reader.join(self.location, self.dest_id, self.from_time)
        #self.reader.join(pyyjj.location(kfj.MODES['live'], kfj.CATEGORIES['md'], 'xtp', 'xtp', ctx.locator) , 0, self.from_time)

        self.logger = create_logger("converter", ctx.log_level, self.location)

        self.logger.info("converter creation success!")

    def read_data_source(self):
        # transfer csv to df
        return pd.read_csv(open(self.filepath))

    def write_data_to_journal(self):
        df = self.read_data_source()

        # mark SessionStart
        # session start time should be smaller than the first quote, and greater than 0
        self.writer.mark_with_time(int(df.loc[0,:].Timestamp * 1e6 + 1.4517500485e18 - 1e6), kfm.SessionStart)
        #self.writer.mark_with_time(1, kfm.SessionStart)
        
        for i in range(len(df)):
            tick_data = df.loc[i,:]
            quote = pywingchun.Quote()

            # process quote data
            self.data_to_quote(tick_data, quote)

            # write to journal
            quote.__write__(quote.data_time, self.writer)

        # mark SessionEnd
        self.writer.mark_with_time(int(df.loc[len(df)-1,:].Timestamp * 1e6 + 1.4517500485e18), kfm.SessionEnd)
        self.logger.info("write journal success!")

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

    def data_to_quote(self, data, quote):
        quote.set_source_id("backtest")
        quote.set_trading_day(data.TradingDay)
        quote.data_time = int(data.Timestamp * 1e6 + 1.4517500485e18)
        quote.set_instrument_id(data.InstrumentID)
        quote.set_exchange_id(data.Exchange)
        #quote.instrument_type = InstrumentType.Future 是一个int8的枚举类型
        quote.pre_close_price = data.PreClosePrice
        # quote.pre_settlement_price = 0
        quote.last_price = data.LastPrice
        quote.volume = int(data.Volume)
        quote.turnover = data.Turnover
        quote.pre_open_interest = data.PreOpenInterest
        quote.open_interest = data.OpenInterest
        quote.open_price = data.OpenPrice
        quote.high_price = data.HighPrice
        quote.low_price = data.LowPrice
        quote.upper_limit_price = data.UpperLimitPrice
        quote.lower_limit_price = data.LowerLimitPrice
        quote.close_price = data.ClosePrice
        # quote.settlement_price = 0
        quote.set_bid_price([data.BidPrice1, data.BidPrice2, data.BidPrice3, data.BidPrice4, data.BidPrice5])
        quote.set_ask_price([data.AskPrice1, data.AskPrice2, data.AskPrice3, data.AskPrice4, data.AskPrice5])
        quote.set_bid_volume([int(vol) for vol in [data.BidVolume1, data.BidVolume2, data.BidVolume3, data.BidVolume4, data.BidVolume5]])
        quote.set_ask_volume([int(vol) for vol in [data.AskVolume1, data.AskVolume2, data.AskVolume3, data.AskVolume4, data.AskVolume5]])
