
import pywingchun
from rx.subject import Subject
import json

class AlgoOrder(pywingchun.AlgoOrder):
    registry = {}

    __params_schema__ = None
    __status_schema__ = None

    def __init__(self, **kwargs):
        order_id = kwargs.pop("order_id", 0)
        self.sender_uid = kwargs.pop("sender_uid", 0)
        self.active_orders = {}
        self.active = False
        pywingchun.AlgoOrder.__init__(self, order_id)
        self.subject = Subject()

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry[cls.__name__] = cls

    def __repr__(self):
        return '%s(%r,%r)' % (self.__class__, self.params, self.status)

    @classmethod
    def types(cls):
        return list(cls.registry.keys())

    def dumps(self):
        dct = {"order_id": self.order_id, "sender_uid": self.sender_uid , "algo_type": self.type, "status": self.status, "params": self.params, "active": self.active}
        return json.dumps(dct)

    @classmethod
    def create(cls, type, **kwargs):
        return cls.registry[type](**kwargs)

    def on_start(self, ctx):
        self.active = True

    def on_stop(self, ctx):
        self.active = False

    def on_modify(self, ctx, msg):
        raise NotImplementedError

    def on_child_trade(self, ctx, trade):
        raise NotImplementedError

    def on_child_order(self, ctx, order):
        if self.active:
            order_id = order.order_id
            if order_id in self.active_orders:
                pass

    def on_quote(self, ctx, quote):
        raise NotImplementedError

    def on_order_report(self, ctx, report_msg):
        raise NotImplementedError

    @property
    def type(self):
        return self.__class__.__name__

    @property
    def sent(self):
        return self.order_id > 0

    def send_notice(self):
        print("send notice .......")
        self.subject.on_next(self)


