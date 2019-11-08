
import pywingchun
from rx.subject import Subject
import json

class AlgoOrder(pywingchun.AlgoOrder):
    registry = {}

    __params_schema__ = None
    __status_schema__ = None

    def __init__(self, ctx, **kwargs):
        self.ctx = ctx
        order_id = kwargs.pop("order_id", 0)
        pywingchun.AlgoOrder.__init__(self, order_id)
        self.subject = Subject()

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry[cls.__name__] = cls

    @classmethod
    def types(cls):
        return list(cls.registry.keys())

    def dumps(self):
        dct = {"order_id": self.order_id, "algo_type": self.__class__.__name__, "status": self.status, "params": self.params, "active": self.active}
        return json.dumps(dct)

    @classmethod
    def create(cls, type, **kwargs):
        return cls.registry[type](**kwargs)

    def on_start(self, ctx):
        self.started = True

    def on_child_trade(self, ctx, trade):
        raise NotImplementedError

    def on_child_order(self, ctx, order):
        raise NotImplementedError

    def on_quote(self, ctx, quote):
        raise NotImplementedError

    def on_order_status(self, ctx, status):
        pass

    @property
    def active(self):
        return False

    @property
    def sent(self):
        return self.order_id > 0

    def send_notice(self):
        self.subject.on_next(self.dumps())
