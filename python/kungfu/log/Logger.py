import logging
import nnpy
import pyyjj

LOG_MSG_FORMAT = '[%(nanotime)s] [%(levelname)s] [pid/tid %(process)6d/%(tid)-6d] [%(filename)s:%(lineno)d#%(funcName)s] %(message)s'
LOG_DATE_FORMAT = '%Y-%m-%d %H:%M:%S'

LOG_LEVELS = {
    'trace':logging.DEBUG,
    'debug':logging.DEBUG,
    'info':logging.INFO,
    'warning':logging.WARN,
    'error':logging.ERROR,
    'critical':logging.CRITICAL,
}

class KungfuFormatter(logging.Formatter):
    def format(self, record):
        time = pyyjj.nano_time()
        nano = time % 1000000000
        if record.levelname == 'WARN':
            record.levelname = 'warning'
        record.levelname = '{:^8}'.format(record.levelname.lower())
        record.nanotime = '{}.{:09d}'.format(pyyjj.parse_nano(time, LOG_DATE_FORMAT), nano)
        record.tid = pyyjj.thread_id()
        return logging.Formatter.format(self, record)

def create_logger(name, level):
    formatter = KungfuFormatter(LOG_MSG_FORMAT)
    handler = logging.StreamHandler()
    handler.setLevel(LOG_LEVELS[level])
    handler.setFormatter(formatter)

    logger = logging.getLogger(name)
    logger.setLevel(LOG_LEVELS[level])
    logger.addHandler(handler)
    return logger
