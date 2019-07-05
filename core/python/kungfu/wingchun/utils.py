
import pywingchun
from kungfu.wingchun.constants import *
def to_dict(obj):
    d = {}
    for attr in dir(obj):
        if not attr.startswith('__'):
            if attr == "order_id" or attr == "parent_id" or attr == "parent_order_id" or attr == "trade_id":
                d[attr] = str(getattr(obj, attr))
            elif attr in ["price_type", "instrument_type", "offset", "status", "side", "direction", "time_condition", "volume_condition"]:
                d[attr] = int(getattr(obj, attr))
            else:
                d[attr] = getattr(obj, attr)
    return d

def get_instrument_type(instrument_id, exchange_id):
    if exchange_id == Exchange.SSE or exchange_id == Exchange.SZE:
        return InstrumentType.Stock
    else:
        return InstrumentType.Future

is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id