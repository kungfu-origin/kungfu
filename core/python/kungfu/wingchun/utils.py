
import pywingchun
from kungfu.wingchun.constants import *
import json

get_instrument_type = pywingchun.utils.get_instrument_type
is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id

def is_final_status(order_status):
    return int(order_status) in AllFinalOrderStatus

def get_position_effect(instrument_type, side, offset):
    if instrument_type in InstrumentTypeInStockAccount:
        return Direction.Long
    elif side == Side.Buy and offset == Offset.Open:
        return Direction.Long
    elif side == Side.Sell and (offset == Offset.Close or offset == Offset.CloseToday or offset == Offset.CloseYesterday):
        return Direction.Long
    elif side == Side.Sell and offset == Offset.Open:
        return Direction.Short
    elif side == Side.Buy and (offset == Offset.Close or offset == Offset.CloseToday or offset == Offset.CloseYesterday):
        return Direction.Short
    else:
        raise ValueError('could not find position effect for instrument_type {}, side {}, offset {}'.format(instrument_type, side, offset))

class WCEncoder(json.JSONEncoder):
    def default(self, obj):
        if type(obj) in ENUM_TYPES:
            return int(obj)
        return json.JSONEncoder.default(self, obj)
