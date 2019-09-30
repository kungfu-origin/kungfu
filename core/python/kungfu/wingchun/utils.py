
import pywingchun
from kungfu.wingchun.constants import *

get_instrument_type = pywingchun.utils.get_instrument_type
is_valid_price = pywingchun.utils.is_valid_price
get_symbol_id = pywingchun.utils.get_symbol_id

def object_as_dict(obj):
    d = {}
    for attr in dir(obj):
        if not attr.startswith('__'):
            try:
                value = getattr(obj, attr)
            except UnicodeDecodeError:
                continue
            if type(value) in ENUM_TYPES:
                d[attr] = int(value)
            else:
                d[attr] = value
    return d

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

def min_order_volume(instrument_id, exchange_id):
    instrument_type = get_instrument_type(instrument_id, exchange_id)
    return 100 if instrument_type in InstrumentTypeInStock else 1

def get_class_from_msg_type(msg_type):
    if msg_type == pywingchun.constants.MsgType.Quote:
        return pywingchun.Quote
    elif msg_type == pywingchun.constants.MsgType.Transaction:
        return pywingchun.Transaction
    elif msg_type == pywingchun.constants.MsgType.Entrust:
        return pywingchun.Entrust
    elif msg_type == pywingchun.constants.MsgType.OrderInput:
        return pywingchun.OrderInput
    elif msg_type == pywingchun.constants.MsgType.OrderAction:
        return pywingchun.OrderAction
    elif msg_type == pywingchun.constants.MsgType.Order:
        return pywingchun.Order
    elif msg_type == pywingchun.constants.MsgType.Trade:
        return pywingchun.Trade
    elif msg_type == pywingchun.constants.MsgType.Position:
        return pywingchun.Position
    elif msg_type == pywingchun.constants.MsgType.Asset:
        return pywingchun.Asset
    elif msg_type == pywingchun.constants.MsgType.PositionDetail:
        return pywingchun.PositionDetail
    elif msg_type == pywingchun.constants.MsgType.Instrument:
        return pywingchun.Instrument
    else:
        raise ValueError("invalid msg_type {}".format(msg_type))

def get_data(event):
    if event.msg_type == pywingchun.constants.MsgType.Quote:
        return pywingchun.utils.get_quote(event)
    elif event.msg_type == pywingchun.constants.MsgType.Entrust:
        return pywingchun.utils.get_entrust(event)
    elif event.msg_type == pywingchun.constants.MsgType.Transaction:
        return pywingchun.utils.get_transaction(event)
    elif event.msg_type == pywingchun.constants.MsgType.Bar:
        return pywingchun.utils.get_bar(event)
    elif event.msg_type == pywingchun.constants.MsgType.OrderInput:
        return pywingchun.utils.get_order_input(event)
    elif event.msg_type == pywingchun.constants.MsgType.OrderAction:
        return pywingchun.utils.get_order_action(event)
    elif event.msg_type == pywingchun.constants.MsgType.Order:
        return pywingchun.utils.get_order(event)
    elif event.msg_type == pywingchun.constants.MsgType.Trade:
        return pywingchun.utils.get_trade(event)
    elif event.msg_type == pywingchun.constants.MsgType.Position:
        return pywingchun.utils.get_position(event)
    elif event.msg_type == pywingchun.constants.MsgType.PositionDetail:
        return pywingchun.utils.get_position_detail(event)
    elif event.msg_type == pywingchun.constants.MsgType.Asset:
        return pywingchun.utils.get_asset(event)
    else:
        return None

def write_data(writer, trigger_time, data):
    if type(data) is pywingchun.Quote:
        return pywingchun.utils.write_quote(writer, trigger_time, data)
    elif type(data) is pywingchun.Entrust:
        return pywingchun.utils.write_entrust(writer, trigger_time, data)
    elif type(data) is pywingchun.Transaction:
        return pywingchun.utils.write_transaction(writer, trigger_time, data)
    elif type(data) is pywingchun.Bar:
        return pywingchun.utils.write_bar(writer, trigger_time, data)
    elif type(data) is pywingchun.OrderInput:
        return pywingchun.utils.write_order_input(writer, trigger_time, data)
    elif type(data) is pywingchun.OrderAction:
        return pywingchun.utils.write_order_action(writer, trigger_time, data)
    elif type(data) is pywingchun.Order:
        return pywingchun.utils.write_order(writer, trigger_time, data)
    elif type(data) is pywingchun.Trade:
        return pywingchun.utils.write_trade(writer, trigger_time, data)
    elif type(data) is pywingchun.Position:
        return pywingchun.utils.write_position(writer, trigger_time, data)
    elif type(data) is pywingchun.Asset:
        return pywingchun.utils.write_asset(writer, trigger_time, data)
    elif type(data) is pywingchun.PositionDetail:
        return pywingchun.utils.write_position_detail(writer, trigger_time, data)
    elif type(data) is pywingchun.Instrument:
        return pywingchun.utils.write_instrument(writer, trigger_time, data)
    else:
        raise ValueError("invalid data type {}".format(type(data)))

def write_data_with_time(writer, gen_time, data):
    if type(data) is pywingchun.Quote:
        return pywingchun.utils.write_quote_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Entrust:
        return pywingchun.utils.write_entrust_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Transaction:
        return pywingchun.utils.write_transaction_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Bar:
        return pywingchun.utils.write_bar_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.OrderInput:
        return pywingchun.utils.write_order_input_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.OrderAction:
        return pywingchun.utils.write_order_action_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Order:
        return pywingchun.utils.write_order_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Trade:
        return pywingchun.utils.write_trade_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Position:
        return pywingchun.utils.write_position_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Asset:
        return pywingchun.utils.write_asset_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.PositionDetail:
        return pywingchun.utils.write_position_detail_with_time(writer, gen_time, data)
    elif type(data) is pywingchun.Instrument:
        return pywingchun.utils.write_instrument_with_time(writer, gen_time, data)
    else:
        raise ValueError("invalid data type {}".format(type(data)))

def flatten_json(y):
    out = {}
    def flatten(x, name=''):
        if type(x) is dict:
            for a in x:
                flatten(x[a], name + a + '_')
        elif type(x) is list:
            i = 0
            for a in x:
                flatten(a, name + str(i) + '_')
                i += 1
        else:
            out[name[:-1]] = x
    flatten(y)
    return out

def get_csv_header(msg_type):
    cls = get_class_from_msg_type(msg_type)
    obj = cls()
    d = object_as_dict(obj)
    return flatten_json(d).keys()


