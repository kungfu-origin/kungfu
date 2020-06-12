import csv
import inspect
import glob
import os
from kungfu.yijinjing.locator import Locator
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj


class Adapter:
    def __init__(self, ctx):
        self.ctx = ctx
        self.named_types = {}
        self.tagged_types = {}

        lf_states = inspect.getmembers(lf.state, inspect.isclass)
        lf_types = inspect.getmembers(lf.types, inspect.isclass)
        states = set(map(lambda s: s[0], lf_states))
        for t in filter(lambda m: m[1].__has_data__ and m[0] in states, lf_types):
            self.named_types[t[0]] = t[1]
            self.tagged_types[t[1].__tag__] = t[1]

    def write_data(self):
        raise NotImplemented

    def run(self):
        self.ctx.logger.info('exporting data to csv')
        self.write_data()
        sink = yjj.null_sink()
        locators = {}
        writers = {}
        for csv_file in glob.glob(os.path.join(self.ctx.inbox_dir, '*.csv')):
            filename = os.path.splitext(os.path.basename(csv_file))[0]
            name_list = filename.split('.')
            if len(name_list) != 4:
                self.ctx.logger.error(f'invalid csv name {filename}')
                continue
            category, group, name, type_name = name_list
            writer_key = category + group + name
            if writer_key not in writers:
                locator = Locator(self.ctx.dataset_dir)
                locators[writer_key] = locator
                home = yjj.location(lf.enums.mode.DATA, lf.enums.get_category_by_name(category), group, name, locator)
                writers[writer_key] = yjj.writer(home, 0, True, sink.publisher)
            writer = writers[writer_key]
            data_type = self.named_types[type_name]
            with open(csv_file, 'r', newline='') as file:
                reader = csv.reader(file)
                header = next(reader)
                for row in reader:
                    json_text = '{' + ','.join([f'"{t[0]}":{t[1]}' for t in zip(header, row)]) + '}'
                    data = data_type(json_text)
                    writer.write(0, data)
