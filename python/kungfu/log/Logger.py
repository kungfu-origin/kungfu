import logging
import os
import nnpy, pyyjj

LOG_MSG_FORMAT = '[%(nanotime)s] [%(loglevel)s] [pid/tid %(process)6d/%(tid)-6d] [%(filename)s:%(lineno)d#%(funcName)s] %(message)s'
LOG_DATE_FORMAT = '%Y-%m-%d %H:%M:%S'
LOG_FILE_DATEEXT_FORMAT = '%Y-%m-%d'

LOG_LEVELS = {
    'trace': logging.DEBUG,
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARNING,
    'warning': logging.WARNING,
    'error': logging.ERROR,
    'critical': logging.CRITICAL,
}

class ansicolor:
    # Formatting codes
    reset = "\033[m"
    bold = "\033[1m"
    dark = "\033[2m"
    underline = "\033[4m"
    blink = "\033[5m"
    reverse = "\033[7m"
    concealed = "\033[8m"
    clear_line = "\033[K"
    # Foreground colors
    black = "\033[30m"
    red = "\033[31m"
    green = "\033[32m"
    yellow = "\033[33m"
    blue = "\033[34m"
    magenta = "\033[35m"
    cyan = "\033[36m"
    white = "\033[37m"
    # Background colors
    on_black = "\033[40m"
    on_red = "\033[41m"
    on_green = "\033[42m"
    on_yellow = "\033[43m"
    on_blue = "\033[44m"
    on_magenta = "\033[45m"
    on_cyan = "\033[46m"
    on_white = "\033[47m"

COLORS = {
    "debug": ansicolor.cyan,
    "info": ansicolor.green,
    "warn": ansicolor.yellow + ansicolor.bold,
    "warning": ansicolor.yellow + ansicolor.bold,
    "error": ansicolor.red + ansicolor.bold,
    "critical": ansicolor.bold + ansicolor.on_red
}

class KungfuFormatter(logging.Formatter):
    def format_level(self, levelname):
        return '{:^8}'.format(levelname)

    def format(self, record):
        time = pyyjj.nano_time()
        nano = time % 1000000000
        record.loglevel = self.format_level(record.levelname.lower())
        record.nanotime = '{}.{:09d}'.format(pyyjj.parse_nano(time, LOG_DATE_FORMAT), nano)
        record.tid = pyyjj.thread_id()
        return logging.Formatter.format(self, record)

class ColorFormatter(KungfuFormatter):
    def format_level(self, levelname):
        return '{}{:^8}{}'.format(COLORS[levelname], levelname, ansicolor.reset)

def create_logger(name, level):
    base_dir = os.getenv('KF_HOME')
    log_archive_folder = os.path.join(base_dir, 'log', 'archive')
    if not os.path.exists(log_archive_folder):
        os.makedirs(log_archive_folder)
    time = pyyjj.nano_time()
    log_dateext = pyyjj.parse_nano(time, LOG_FILE_DATEEXT_FORMAT)
    log_path = os.path.join(log_archive_folder, '{}_py_{}.log'.format(name, log_dateext))

    file_handler = logging.FileHandler(log_path)
    file_handler.setFormatter(KungfuFormatter(LOG_MSG_FORMAT))

    console_handler = logging.StreamHandler()
    console_handler.setFormatter(ColorFormatter(LOG_MSG_FORMAT))

    logger = logging.getLogger(name)
    logger.setLevel(LOG_LEVELS[level])
    logger.addHandler(file_handler)
    logger.addHandler(console_handler)
    return logger
