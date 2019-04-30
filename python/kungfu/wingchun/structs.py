import ctypes, json


class FutureInstrument(ctypes.Structure):
    _fields_ = [
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('instrument_type', ctypes.c_char),
        ('product_id', ctypes.c_char * 32),
        ('contract_multiplier', ctypes.c_int),
        ('price_tick', ctypes.c_double),
        ('open_date', ctypes.c_char * 9),
        ('create_date', ctypes.c_char * 9),
        ('expire_date', ctypes.c_char * 9),
        ('delivery_year', ctypes.c_int),
        ('delivery_month', ctypes.c_int),
        ('is_trading', ctypes.c_bool),
        ('long_margin_ratio', ctypes.c_double),
        ('short_margin_ratio', ctypes.c_double)
    ]

class Quote(ctypes.Structure):
    _fields_ = [
        ('source_id', ctypes.c_char * 16),
        ('trading_day', ctypes.c_char * 9),
        ("rcv_time", ctypes.c_longlong),
        ('data_time', ctypes.c_longlong),
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('instrument_type', ctypes.c_char),
        ('pre_close_price', ctypes.c_double),
        ('pre_settlement_price', ctypes.c_double),
        ('last_price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('turnover', ctypes.c_double),
        ('pre_open_interest', ctypes.c_double),
        ('open_interest', ctypes.c_double),
        ('open_price', ctypes.c_double),
        ('high_price', ctypes.c_double),
        ('low_price', ctypes.c_double),
        ('upper_limit_price', ctypes.c_double),
        ('lower_limit_price', ctypes.c_double),
        ('close_price', ctypes.c_double),
        ('settlement_price', ctypes.c_double),
        ('bid_price', ctypes.c_double * 10),
        ('ask_price', ctypes.c_double * 10),
        ('bid_volume', ctypes.c_longlong * 10),
        ('ask_volume', ctypes.c_longlong * 10)
    ]

class Entrust(ctypes.Structure):
    _fields_ = [
        ('source_id', ctypes.c_char * 16),
        ('trading_day', ctypes.c_char * 9),
        ("rcv_time", ctypes.c_longlong),
        ('data_time', ctypes.c_longlong),
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('instrument_type', ctypes.c_char),
        ('price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('side', ctypes.c_char),
        ('price_type', ctypes.c_char),
        ('main_seq', ctypes.c_longlong),
        ('seq', ctypes.c_longlong)
    ]

class Transaction(ctypes.Structure):
    _fields_ = [
        ('source_id', ctypes.c_char * 16),
        ('trading_day', ctypes.c_char * 9),
        ("rcv_time", ctypes.c_longlong),
        ('data_time', ctypes.c_longlong),
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('instrument_type', ctypes.c_char),
        ('price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('bid_no', ctypes.c_longlong),
        ('ask_no', ctypes.c_longlong),
        ('exec_type', ctypes.c_char),
        ('bs_flag', ctypes.c_char),
        ('main_seq', ctypes.c_longlong),
        ('seq', ctypes.c_longlong)
    ]

class OrderInput(ctypes.Structure):
    _fields_ = [
        ('order_id', ctypes.c_longlong),
        ('instrument_id', ctypes.c_char_p),
        ('exchange_id', ctypes.c_char_p),
        ('account_id', ctypes.c_char_p),
        ('client_id', ctypes.c_char_p),
        ('instrument_type', ctypes.c_char),
        ('limit_price', ctypes.c_double),
        ('frozen_price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('side', ctypes.c_char),
        ('offset', ctypes.c_char),
        ('price_type', ctypes.c_char),
        ('volume_condition', ctypes.c_char),
        ('time_condition', ctypes.c_char),
        ('parent_id', ctypes.c_longlong)
    ]


class Order(ctypes.Structure):
    _fields_ = [
        ('rcv_time', ctypes.c_longlong),
        ('order_id', ctypes.c_longlong),
        ('insert_time', ctypes.c_longlong),
        ('update_time', ctypes.c_longlong),
        ('trading_day', ctypes.c_char * 9),
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('account_id', ctypes.c_char * 32),
        ('client_id', ctypes.c_char * 32),
        ('instrument_type', ctypes.c_char),
        ('limit_price', ctypes.c_double),
        ('frozen_price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('volume_traded', ctypes.c_longlong),
        ('volume_left', ctypes.c_longlong),
        ('tax', ctypes.c_double),
        ('commission', ctypes.c_double),
        ('status', ctypes.c_char),
        ('error_id', ctypes.c_int),
        ('error_msg', ctypes.c_char * 32),
        ('parent_id', ctypes.c_longlong),
        ('side', ctypes.c_char),
        ('offset', ctypes.c_char),
        ('price_type', ctypes.c_char),
        ('volume_condition', ctypes.c_char),
        ('time_condition', ctypes.c_char)
    ]

class Trade(ctypes.Structure):
    _fields_ = [
        ("rcv_time", ctypes.c_longlong),
        ('id', ctypes.c_longlong),
        ('order_id', ctypes.c_longlong),
        ('parent_order_id', ctypes.c_longlong),
        ('trade_time', ctypes.c_longlong),
        ('instrument_id', ctypes.c_char * 32),
        ('exchange_id', ctypes.c_char * 16),
        ('account_id', ctypes.c_char * 32),
        ('client_id', ctypes.c_char * 32),
        ('instrument_type', ctypes.c_char),
        ('side', ctypes.c_char),
        ('offset', ctypes.c_char),
        ('price', ctypes.c_double),
        ('volume', ctypes.c_longlong),
        ('tax', ctypes.c_double),
        ('commission', ctypes.c_double),
    ]
