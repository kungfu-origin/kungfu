import pyyjj
import pywingchun

import kungfu.yijinjing.msg as kfm
import kungfu.yijinjing.time as kft
import kungfu.yijinjing.journal as kfj
from kungfu.yijinjing.log import create_logger
from kungfu.command.history.converter import Converter

from extensions import EXTENSION_REGISTRY_BT

import pandas as pd
import sys


class csvConverter(Converter):

    def __init__(self, ctx):
        super(csvConverter, self).__init__(ctx)
        self.filepath = ctx.arguments['file_path']
        self.logger.info("converter creation success!")

    def read_data_source(self):
        # transfer csv to df
        return pd.read_csv(open(self.filepath))

    def write_data_to_journal(self):
        df = self.read_data_source()
        self.mark_SessionStart(int(df.loc[0,:].Timestamp * 1e6 + 1.4517500485e18 - 1e6))

        for i in range(len(df)):
            tick_data = df.loc[i,:]
            quote = pywingchun.Quote()
            self.data_to_quote(tick_data, quote) # process quote data
            quote.__write__(quote.data_time, self.writer) # write to journal

        self.mark_SessionEnd(int(df.loc[len(df)-1,:].Timestamp * 1e6 + 1.4517500485e18))
        self.logger.info("write journal success!")

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

EXTENSION_REGISTRY_BT.register_extension('csvConverter', csvConverter)

# tar -zcvf csvConverter-v0.0.1.tgz package
# yarn dev -l trace extension install -f /Users/yetaoran/Documents/kungfu_new/kungfu/core/python/kungfu/command/history/csvConverter/csvConverter-v0.0.1.tgz 