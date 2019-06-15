import os
import glob
import re
import pyyjj


JOURNAL_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}'.format(r'(.*)', os.sep,  # literal 'journal'
                                                     r'(.*)', os.sep,  # mode
                                                     r'(.*)', os.sep,  # category
                                                     r'(.*)', os.sep,  # group
                                                     r'yjj.(.*).(\d+).journal',  # name + page_id
                                                     )
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)


MODES = {
    'live': pyyjj.mode.LIVE,
    'data': pyyjj.mode.DATA,
    'replay': pyyjj.mode.REPLAY,
    'backtest': pyyjj.mode.BACKTEST,
    '*': None
}

CATEGORIES = {
    'md': pyyjj.category.MD,
    'td': pyyjj.category.TD,
    'strategy': pyyjj.category.STRATEGY,
    'system': pyyjj.category.SYSTEM,
    '*': None
}


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
