
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
    
get_instrument_type = pywingchun.utils.get_instrument_type
is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id