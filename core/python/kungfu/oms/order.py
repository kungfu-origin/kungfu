
import pywingchun

class OrderBase:
    def __init__(self, ctx, **kwargs):
        self.ctx = ctx
        self._order_id = kwargs.pop("order_id", None)

    @property
    def is_algo(self):
        raise NotImplementedError

    @property
    def is_active(self):
        raise NotImplementedError

    @property
    def order_id(self):
        return self._order_id

    @order_id.setter
    def order_id(self, order_id):
        self._order_id = order_id

    @property
    def triggered(self):
        return self._order_id is not None

    def execute(self):
        raise NotImplementedError

    def cancel(self):
        raise NotImplementedError

    def on_order(self, event, order):
        raise NotImplementedError

    def on_trade(self, event, trade):
        raise NotImplementedError

    def on_quote(self, event, quote):
        raise NotImplementedError

class SimpleOrder(OrderBase):
    def __init__(self, ctx, **kwargs):
        super(SimpleOrder, self).__init__(ctx, **kwargs)

        self._trading_day = kwargs.pop("trading_day", None)

        self._insert_time = kwargs.pop("insert_time", None)
        self._update_time = kwargs.pop("update_time", None)

        self._instrument_id = kwargs.pop("instrument_id", None)
        self._exchange_id = kwargs.pop("exchange_id", None)

        self._account_id = kwargs.pop("account_id", None)
        self._client_id = kwargs.pop("client_id", None)

        self._limit_price = kwargs.pop("limit_price", None)
        self._frozen_price = kwargs.pop("frozen_price", None)

        self._volume = kwargs.pop("volume", None)
        self._volume_traded = kwargs.pop("volume_traded", None)
        self._volume_left = kwargs.pop("volume_left", None)

        self._tax = kwargs.pop("tax", None)
        self._commission = kwargs.pop("commission", None)

        self._status = kwargs.pop("status", None)

        self._error_id = kwargs.pop("error_id", None)
        self._error_msg = kwargs.pop("error_msg", None)

        self._side = kwargs.pop("side", None)
        self._offset = kwargs.pop("offset", None)
        self._price_type = kwargs.pop("price_type", None)
        self._volume_condition = kwargs.pop("volume_condition", None)
        self._time_condition = kwargs.pop("time_condition", None)

        self._parent_id = kwargs.pop("parent_id", None)

    @property
    def account_id(self):
        return self._account_id

    @account_id.setter
    def account_id(self, account_id):
        self._account_id = account_id

    @property
    def parent_id(self):
        return self._parent_id

    @parent_id.setter
    def parent_id(self, parent_id):
        self._parent_id = parent_id

    @property
    def is_child_order(self):
        return self.parent_id > 0

    @property
    def instrument_id(self):
        return self._instrument_id

    @instrument_id.setter
    def instrument_id(self, instrument_id):
        self._instrument_id = instrument_id

    def execute(self):
        if not self.triggered:
            input = pywingchun.OrderInput()
            if self._instrument_id: input.instrument_id = self._instrument_id
            if self._exchange_id: input.exchange_id = self._exchange_id
            if self._account_id: input.account_id = self._account_id
            if self._limit_price: input.limit_price = self._limit_price
            if self._volume: input.volume = self._volume
            if self._side: input.side = self._side
            if self._offset: input.offset = self._offset
            if self._price_type: input.price_type = self._price_type
            if self._parent_id: input.parent_id = self._parent_id
            self.order_id = self.ctx.insert_order(input)

    def cancel(self):
        if self.is_active:
            self.ctx.cancel_order(self.order_id)

    def on_quote(self, event, quote):
        pass

    def on_order(self, event, order):
        if order.order_id == self.order_id:
            self._status = order.status
            self._update_time = order.update_time
            self._volume = order.volume
            self._volume_traded = order.volume_traded
            self._volume_left = order.volume_left

    def on_trade(self, event, trade):
        pass

class AlgoOrder(OrderBase):
    registry = {}
    def __init__(self, ctx, **kwargs):
        super(AlgoOrder, self).__init__(ctx, **kwargs)

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry[cls.__name__] = cls

    @classmethod
    def factory(cls, type, **kwargs):
        return cls.registry[type](**kwargs)

    @property
    def type(self):
        return self.__class__.__name__

    def is_algo(self):
        return True

    @property
    def message(self):
        raise NotImplementedError

class TwapOrder(AlgoOrder):
    def __init__(self, ctx, **kwargs):
        super(TwapOrder, self).__init__(ctx, **kwargs)

    def on_quote(self, event, quote):
        pass

    def on_order(self, event, quote):
        pass

    def on_trade(self, event, trade):
        pass



