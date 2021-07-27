import kungfu

from kungfu.data.adapter import Adapter
from kungfu.yijinjing.sinks.csv import open_csv_sink

lf = kungfu.__bindings__.longfist
yjj = kungfu.__bindings__.yijinjing


class KfaAdapter(Adapter):
    def __init__(self, ctx):
        super().__init__(ctx)

    def write_data(self):
        with open_csv_sink(self.ctx, self.tagged_types) as csv_sink:
            yjj.assemble([self.ctx.runtime_locator]) >> csv_sink
