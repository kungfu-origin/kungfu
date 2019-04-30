import pyyjj


class TaskScheduler:
    def __init__(self):
        self._nano = 0
        self._started = False
        self._cbs = []
        self._cb_id = 0

    def run(self):
        if not self._started:
            self._started = True

    def stop(self):
        if self._started:
            self._started = False

    def insert_callback_at(self, nano, callback):
        cb_id = self.__get_id()
        self._cbs.append((cb_id, nano, callback))
        return cb_id

    def insert_callback_after(self, msec, callback):
        return self.insert_callback_at(self._nano + msec * 1000000, callback)

    def insert_callback_at_next(self, time_str, callback):
        cur_date_str = pyyjj.parse_nano(self._nano, "%Y%m%d-");
        date_time_str = cur_date_str + time_str;
        nano = pyyjj.parse_time(date_time_str, "%Y%m%d-%H:%M:%S");
        if nano < self._nano:
            nano += 24 * 60 * 60 * 1000000000
        return self.insert_callback_at(nano, callback)

    def update_nano(self, nano):
        if nano < self._nano:
            pass
        self._nano = nano
        i = 0
        while i < len(self._cbs):
            if self._cbs[i][1] <= self._nano:
                self._cbs[i][2](self._nano)
                self._cbs.pop(i)
            else:
                i += 1

    def get_nano(self):
        return self._nano

    def __get_id(self):
        self._cb_id += 1
        return self._cb_id
