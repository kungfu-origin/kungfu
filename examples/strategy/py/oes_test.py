import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

source = "oes"
account = "customer167"
tickers = ["600000"]
#tickers = ["603993","603986","603833","603799","603288","603260","603259"]
#tickers = ["603993","603986","603833","603799","603288","603260","603259"]
price = 11.0
volume = 300
exchange = Exchange.SSE

def pre_start(context):
    context.add_account(source, account, 100000000.0)
    context.subscribe(source, [ticker], exchange)
    context.canceled = False
    context.count = 0;
    pass

def pre_run(context):
    context.add_timer(1 * 1e9, test_time_interval)
    context.add_timer(1, test_time_interval)
    pass

def test_time_interval(context):
    context.log.info('test time interval without event')

def test_time_interval(context, event):
    context.log.info('test time interval')

def on_quote(context, quote):
    context.log.info("[on_quote] (ticker){} (last_price){} (trading_day){}".format(quote.instrument_id, quote.last_price, quote.trading_day))
	if quote.instrument_id in tickers:
		if (context.count/100) == 0:
		    #order_id = context.insert_limit_order(ticker, exchange, account, quote.last_price, volume, Side.Buy, Offset.Open)
		    order_id = context.insert_order(quote.instrument_id, exchange, account, quote.last_price, volume, PriceType.Limit, Side.Buy, Offset.Open)
		    context.count += 1;
		    if order_id > 0:
		        context.log.info("[order] (rid){} (ticker){}".format(order_id, quote.instrument_id))

def on_transaction(context, transaction):
    context.log_info("[on_transaction] {} {}".format(transaction.instrument_id, transaction.exchange_id))
    pass

def on_entrust(context, entrust):
    context.log_info("[on_entrust] {} {}".format(entrust.instrument_id, entrust.exchange_id))
    pass

def on_order(context, order):
    context.log.warning('[on_order] (ticker){} (order_id){}, (status){}'.format(order.instrument_id, order.order_id, order.status))

def on_trade(context, trade):
    context.log.warning('[on_trade] (ticker){} (order_id){}, (volume){}'.format(trade.instrument_id, trade.order_id, trade.volume))

def pre_quit(context):
    context.log.warning("pre quit")
