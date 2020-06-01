
from kungfu.yijinjing.sinks.csv import open_csv_sink
from pykungfu import yijinjing as yjj


class KfaAdapter:
    def __init__(self, ctx):
        self.ctx = ctx

    def run(self):
        with open_csv_sink(self.ctx) as sink:
            yjj.assemble([self.ctx.runtime_locator]) >> sink
