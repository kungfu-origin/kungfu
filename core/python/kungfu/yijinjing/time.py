from datetime import datetime, timedelta
import re
import pyyjj


DATETIME_FORMAT = '%Y-%m-%d %H:%M:%S.%N'
NANO_PER_SECOND = 1000000000
EPOCH = datetime.fromtimestamp(0)


SESSION_DATETIME_FORMAT = '%Y-%m-%d %H:%M:%S'
DURATION_FORMAT = '%H:%M:%S.%N'
DURATION_TZ_ADJUST = int(timedelta(hours=datetime.fromtimestamp(0).hour).total_seconds() * 1e9)


def strftime(nanotime, format=DATETIME_FORMAT):
    dt = EPOCH + timedelta(microseconds=nanotime/1000)
    normal_format = format.replace("%N", '{:09d}'.format(nanotime % NANO_PER_SECOND))
    return dt.strftime(normal_format)


def strptime(timestr, format=DATETIME_FORMAT):
    nano = re.findall(r'\d{9}', timestr)
    normal_format = format.replace('%N', '')
    normal_timestr = timestr.replace(nano[0], '') if nano else timestr
    nano = int(nano[0]) if nano else 0
    dt = datetime.strptime(normal_timestr, normal_format)
    second_part = int((dt - EPOCH).total_seconds() * NANO_PER_SECOND)  # round here to avoid precision diff
    return second_part + nano


def strfnow(format=DATETIME_FORMAT):
    return strftime(pyyjj.now_in_nano(), format)
