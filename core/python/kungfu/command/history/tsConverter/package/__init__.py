import pywingchun
import kungfu.yijinjing.time as kft
from kungfu.command.history.converter import Converter
from extensions import EXTENSION_REGISTRY_BT

import os
import json
import time
import tushare as ts

##############################
# available api （积分<=120）:
# daily           日线行情
# new_share       IPO新股列表
# trade_cal       期货交易日历
# opt_basic       期权合约列表
# opt_daily       期权日线行情
# SHIBOR利率数据   shibor
# SHIBOR报价数据   shibor_quote
# LPR贷款基础利率   shibor_lpr
# LIBOR拆借利率    libor
# HIBOR拆借利率    hibor
# cctv_news       新闻联播文字稿
##############################

class tsConverter(Converter):

    def __init__(self, ctx):
        super(tsConverter, self).__init__(ctx)
        self.config_file = os.path.join(ctx.home, 'config.json')
        self.token = json.loads(open(self.config_file).read())['plug-ins'][self.ctx.group]['token']
        self.ts_pro = ts.pro_api(self.token) #eb8dbad659d8467f46b318c4047fe30b71db86d46f393b957c14060b

    def get_data_from_source(self):
        retry_times = 3
        pause = 2
        for i in range(retry_times):
            try:
                return self.ts_pro.query('daily', **self.ctx.arguments) # ts_code='000001.SZ', start_date='20180701', end_date='20180718'
            except:
                time.sleep(pause)

    def write_data_to_journal(self):
        df = self.get_data_from_source()
        df = df.sort_values(by=['trade_date'], ascending=True).reset_index(drop=True)
        self.mark_SessionStart(kft.strptime(df.loc[0,'trade_date'], '%Y%m%d'))

        for i in range(len(df)):
            tick_data = df.loc[i,:]
            quote = pywingchun.Quote()
            self.data_to_quote(tick_data, quote) # process quote data
            quote.__write__(quote.data_time, self.writer) # write to journal

        self.mark_SessionEnd(kft.strptime(df.loc[len(df)-1,'trade_date'], '%Y%m%d'))
        self.logger.info("write journal success!")

    def data_to_quote(self, data, quote):
        quote.set_source_id("backtest")
        quote.set_trading_day(data.trade_date)
        quote.data_time = kft.strptime(data.trade_date, '%Y%m%d') #TODO
        quote.set_instrument_id(data.ts_code)
        #quote.set_exchange_id(data.Exchange)
        #quote.instrument_type = InstrumentType.Future 是一个int8的枚举类型
        quote.pre_close_price = data.pre_close
        #quote.pre_settlement_price = 0
        quote.last_price = data.close
        quote.volume = int(data.vol * 100)
        quote.turnover = data.amount *1000
        #quote.pre_open_interest = data.PreOpenInterest
        #quote.open_interest = data.OpenInterest
        quote.open_price = data.open
        quote.high_price = data.high
        quote.low_price = data.low
        #quote.upper_limit_price = data.UpperLimitPrice
        #quote.lower_limit_price = data.LowerLimitPrice
        quote.close_price = data.close
        #quote.settlement_price = 0
        #quote.set_bid_price([data.BidPrice1, data.BidPrice2, data.BidPrice3, data.BidPrice4, data.BidPrice5])
        #quote.set_ask_price([data.AskPrice1, data.AskPrice2, data.AskPrice3, data.AskPrice4, data.AskPrice5])
        #quote.set_bid_volume([int(vol) for vol in [data.BidVolume1, data.BidVolume2, data.BidVolume3, data.BidVolume4, data.BidVolume5]])
        #quote.set_ask_volume([int(vol) for vol in [data.AskVolume1, data.AskVolume2, data.AskVolume3, data.AskVolume4, data.AskVolume5]])

EXTENSION_REGISTRY_BT.register_extension('tsConverter', tsConverter)

# tar -zcvf tsConverter-v0.0.1.tgz package
# yarn dev -l trace extension install -f /Users/yetaoran/Documents/kungfu_new/kungfu/core/python/kungfu/command/history/tsConverter/tsConverter-v0.0.1.tgz 
# yarn dev history -g tsConverter importer -a ts_code 000001.SZ -a start_date 20180701 -a end_date 20180718 -a api daily