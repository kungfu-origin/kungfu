from kungfu.data.adapter import Adapter
from kungfu.yijinjing.sinks.csv import open_csv_sink
from pykungfu import yijinjing as yjj


class KfaAdapter(Adapter):
    def __init__(self, ctx):
        super().__init__(ctx)

    def write_data(self):
        with open_csv_sink(self.ctx, self.tagged_types) as sink:
            yjj.assemble([self.ctx.runtime_locator]) >> sink
