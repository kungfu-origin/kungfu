import kungfu.yijinjing.time as kft
from kungfu.wingchun.constants import *

def pre_start(context):
    context.log.info("arguments {}".format(context.arguments))

def post_start(context):
    pass

def on_quote(context, quote):
    context.log.info("--- {}".format(quote.instrument_id))
