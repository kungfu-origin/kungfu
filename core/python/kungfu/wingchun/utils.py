
import pywingchun
from kungfu.wingchun.constants import *

def object_as_dict(obj):
    d = {}
    for attr in dir(obj):
        if not attr.startswith('__'):
            value = getattr(obj, attr)
            if type(value) in [InstrumentType, Side, Offset, OrderStatus, Direction, PriceType, VolumeCondition, TimeCondition, LedgerCategory]:
                d[attr] = int(value)
            else:
                d[attr] = value
    return d

def is_final_status(order_status):
    if int(order_status) in [int(OrderStatus.Submitted), int(OrderStatus.Pending),
                             int(OrderStatus.PartialFilledNotActive), int(OrderStatus.Unknown)]:
        return False
    else:
        return True

get_instrument_type = pywingchun.utils.get_instrument_type
is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id