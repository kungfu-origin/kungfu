import os
import glob
import re
import pyyjj


JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}{}{}'.format(r'(.*)', os.sep,  # literal 'journal'
                                                     r'(.*)', os.sep,  # mode
                                                     r'(.*)', os.sep,  # category
                                                     r'(.*)', os.sep,  # group
                                                     r'(.*)', os.sep,  # name
                                                     r'(\w+).(\d+).journal',  # hash + page_id
                                                     )
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)


MODES = {
    'live': pyyjj.mode.LIVE,
    'data': pyyjj.mode.DATA,
    'replay': pyyjj.mode.REPLAY,
    'backtest': pyyjj.mode.BACKTEST,
    '*': pyyjj.mode.LIVE
}

CATEGORIES = {
    'md': pyyjj.category.MD,
    'td': pyyjj.category.TD,
    'strategy': pyyjj.category.STRATEGY,
    'system': pyyjj.category.SYSTEM,
    '*': pyyjj.category.SYSTEM
}


class Locator(pyyjj.locator):
    def __init__(self, ctx):
        pyyjj.locator.__init__(self)
        self._home = ctx.home

    def make_path(self, parent, filename):
        if not os.path.exists(parent):
            os.makedirs(parent)
        return os.path.join(parent, filename)

    def journal_path(self, location):
        path = os.path.join(self._home, 'journal',
                            pyyjj.get_mode_name(location.mode), pyyjj.get_category_name(location.category), location.group, location.name)
        if not os.path.exists(path):
            os.makedirs(path)
        return path

    def socket_path(self, location):
        path = os.path.join(self._home, 'socket',
                            pyyjj.get_mode_name(location.mode), pyyjj.get_category_name(location.category), location.group, location.name)
        if not os.path.exists(path):
            os.makedirs(path)
        return path

    def log_path(self, location):
        path = os.path.join(self._home, 'log', 'archive')
        if not os.path.exists(path):
            os.makedirs(path)
        return path

    def list_page_id(self, location, dest_id):
        page_ids = []
        for journal in glob.glob(os.path.join(self.journal_path(location), hex(dest_id)[2:] + '.*.journal')):
            match = JOURNAL_LOCATION_PATTERN.match(journal[len(self._home) + 1:])
            if match:
                page_id = match.group(7)
                page_ids.append(page_id)
        return page_ids


def collect_journal_locations(ctx):
    kf_home = ctx.home
    search_path = os.path.join(kf_home, 'journal', ctx.mode, ctx.category, ctx.group, 'yjj.' + ctx.name + '.*.journal')

    locations = {}
    for journal in glob.glob(search_path):
        match = JOURNAL_LOCATION_PATTERN.match(journal[len(kf_home) + 1:])
        if match:
            mode = match.group(2)
            category = match.group(3)
            group = match.group(4)
            name = match.group(5)
            page_id = match.group(6)
            key = mode + category + group + name
            if key in locations:
                locations[key]['page_id'].append(page_id)
            else:
                locations[key] = {
                    'mode': mode,
                    'category': category,
                    'group': group,
                    'name': name,
                    'page_id': [page_id]
                }
            ctx.logger.debug('found journal %s %s %s %s', MODES[mode], CATEGORIES[category], group, name)
        else:
            ctx.logger.warn('unable to match journal file %s to pattern %s', journal, JOURNAL_LOCATION_REGEX)

    return locations
