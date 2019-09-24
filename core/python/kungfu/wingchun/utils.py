
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

def get_class_from_msg_type(msg_type):
    if msg_type == pywingchun.constants.MsgType.Quote:
        return pywingchun.Quote
    elif msg_type == pywingchun.constants.MsgType.Order:
        return pywingchun.Order
    elif msg_type == pywingchun.constants.MsgType.Trade:
        return pywingchun.Trade
    else:
        raise ValueError("invalid msg_type {}".fromat(msg_type))

def is_final_status(order_status):
    return int(order_status) in AllFinalOrderStatus

def get_position_effect(instrument_type, side, offset):
    if instrument_type == InstrumentType.Stock:
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

get_instrument_type = pywingchun.utils.get_instrument_type
is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id

def get_data(frame):
    if frame.msg_type == pywingchun.constants.MsgType.Quote:
        return pywingchun.utils.get_quote(frame)
    elif frame.msg_type == pywingchun.constants.MsgType.Order:
        return pywingchun.utils.get_order(frame)
    elif frame.msg_type == pywingchun.constants.MsgType.Trade:
        return pywingchun.utils.get_trade(frame)
    else:
        return None

def write_data(writer, msg_type, trigger_time, data):
    if msg_type == pywingchun.constants.MsgType.Quote:
        return pywingchun.write_quote(writer, trigger_time, data)
    else:
        raise ValueError("invalid msg_type {}".fromat(msg_type))


