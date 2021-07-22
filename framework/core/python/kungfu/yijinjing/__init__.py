import os
import re
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj

os_sep = re.escape(os.sep)

LAYOUT_LOCATION_REGEX = "{}{}{}{}{}{}{}{}{}{}{}".format(
    r"(.*)",
    os_sep,  # category
    r"(.*)",
    os_sep,  # group
    r"(.*)",
    os_sep,  # name
    r"(.*)",
    os_sep,  # layout
    r"(.*)",
    os_sep,  # mode
    r"(.*)",  # filename
)
LAYOUT_LOCATION_PATTERN = re.compile(LAYOUT_LOCATION_REGEX)

JOURNAL_LOCATION_REGEX = "{}{}{}{}{}{}{}{}{}".format(
    r"(.*)",
    os_sep,  # category
    r"(.*)",
    os_sep,  # group
    r"(.*)",
    os_sep,  # name
    r"journal",
    os_sep,
    r"(.*)",  # mode
)
JOURNAL_LOCATION_PATTERN = re.compile(JOURNAL_LOCATION_REGEX)

JOURNAL_PAGE_REGEX = "{}{}{}{}{}{}{}{}{}{}{}".format(
    r"(.*)",
    os_sep,  # category
    r"(.*)",
    os_sep,  # group
    r"(.*)",
    os_sep,  # name
    r"journal",
    os_sep,  # mode
    r"(.*)",
    os_sep,  # mode
    r"(\w+).(\d+).journal",  # hash + page_id
)
JOURNAL_PAGE_PATTERN = re.compile(JOURNAL_PAGE_REGEX)

LOG_REGEX = "{}{}{}{}{}{}{}{}{}{}{}".format(
    r"(.*)",
    os_sep,  # category
    r"(.*)",
    os_sep,  # group
    r"(.*)",
    os_sep,  # name
    r"log",
    os_sep,  # mode
    r"(.*)",
    os_sep,  # mode
    r"(\w+)_(\d+-\d+-\d+).log",  # log_name + date
)
LOG_PATTERN = re.compile(LOG_REGEX)

LOCATION_UNAME_REGEX = r"(.*)/(.*)/(.*)/(.*)"
LOCATION_PATTERN = re.compile(LOCATION_UNAME_REGEX)

MODES = {
    "live": lf.enums.mode.LIVE,
    "data": lf.enums.mode.DATA,
    "replay": lf.enums.mode.REPLAY,
    "backtest": lf.enums.mode.BACKTEST,
    "*": lf.enums.mode.LIVE,
}

CATEGORIES = {
    "md": lf.enums.category.MD,
    "td": lf.enums.category.TD,
    "strategy": lf.enums.category.STRATEGY,
    "system": lf.enums.category.SYSTEM,
    "*": lf.enums.category.SYSTEM,
}

ARCHIVE_PREFIX = "KFA"
