import json
import kungfu
import re

from kungfu.wingchun.constants import *

wc = kungfu.__bindings__.wingchun

get_instrument_type = wc.utils.get_instrument_type
is_valid_price = wc.utils.is_valid_price
hash_instrument = wc.utils.hash_instrument


def is_final_status(order_status):
    return int(order_status) in AllFinalOrderStatus


def get_position_effect(instrument_type, side, offset):
    if instrument_type in InstrumentTypeInStockAccount:
        return Direction.Long
    elif side == Side.Buy and offset == Offset.Open:
        return Direction.Long
    elif side == Side.Sell and (
        offset == Offset.Close
        or offset == Offset.CloseToday
        or offset == Offset.CloseYesterday
    ):
        return Direction.Long
    elif side == Side.Sell and offset == Offset.Open:
        return Direction.Short
    elif side == Side.Buy and (
        offset == Offset.Close
        or offset == Offset.CloseToday
        or offset == Offset.CloseYesterday
    ):
        return Direction.Short
    else:
        raise ValueError(
            "could not find position effect for instrument_type {}, side {}, offset {}".format(
                instrument_type, side, offset
            )
        )


def get_product_id(instrument_id):
    p = re.compile("([a-zA-Z]+)\d+")
    return p.findall(instrument_id)[0]


class WCEncoder(json.JSONEncoder):
    def default(self, obj):
        if type(obj) in ENUM_TYPES:
            return int(obj)
        return json.JSONEncoder.default(self, obj)
