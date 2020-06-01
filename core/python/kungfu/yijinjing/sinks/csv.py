import csv
import os
import inspect
from contextlib import contextmanager
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj


class CsvSink(yjj.sink):
    def __init__(self, ctx):
        yjj.sink.__init__(self)
        self.ctx = ctx
        self.types = {}
        self.files = {}
        self.writers = {}

        lf_states = inspect.getmembers(lf.state, inspect.isclass)
        lf_types = inspect.getmembers(lf.types, inspect.isclass)
        states = set(map(lambda s: s[0], lf_states))
        for t in filter(lambda m: m[1].__has_data__ and m[0] in states, lf_types):
            self.types[t[1].__tag__] = t[1]

    def put(self, location, dest_id, frame):
        if frame.msg_type not in self.types:
            return
        data_type = self.types[frame.msg_type]
        output = os.path.join(self.ctx.inbox_dir, f'{data_type.__name__}.csv')
        if output not in self.writers:
            self.files[output] = open(output, 'w', newline='')
            self.writers[output] = csv.writer(self.files[output])
            self.writers[output].writerow([m for m in vars(data_type) if not m.startswith('_')])
        data = getattr(frame, data_type.__name__)()
        self.writers[output].writerow([getattr(data, m) for m in vars(data_type) if not m.startswith('_')])

    def close(self):
        [file.close() for file in self.files.values()]


@contextmanager
def open_csv_sink(ctx):
    sink = CsvSink(ctx)
    try:
        yield sink
    finally:
        sink.close()
