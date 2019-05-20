from kungfu.wingchun.constants import Region
import os, nnpy, json, threading, time

def test_callback(logger, current):
    logger.info('new current %d', current)

class CalendarClient:
    def __init__(self, logger):
        self._logger = logger
        self._req_socket = nnpy.Socket(nnpy.AF_SP, nnpy.REQ)
        self._req_socket.connect('ipc://' + os.environ["KF_HOME"] + '/calendar/rep.ipc')
        self._sub_socket = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
        self._sub_socket.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, '')
        self._sub_socket.connect('ipc://' + os.environ["KF_HOME"] + '/calendar/pub.ipc')
        self._current = 0
        self._cbs = []
        self._lock = threading.Lock()

    def run(self):
        self.register_switch_day_callback(test_callback)
        self.get_current_trading_day(True)
        self._logger.info("current trading day %d", self._current)
        self._started = True
        self.__loop()
    
    def stop(self):
        self._started = False
        self._sub_thread.join(3)

    def get_current_trading_day(self, via_net = False):
        if via_net or self._current == 0:
            self.__get_current_via_req()
        return str(self._current)

    def get_next_trading_day(self, start_date='', delta=0):
        if len(start_date) == 0:
            start_date = self.get_current_trading_day()
        j = dict
        j["req"] = "calc"
        j["region"] = Region.CN
        j["start_date"] = start_date
        j["delta"] = delta
        self._req_socket.send(json.dumps(j))
        reply = self._req_socket.recv()
        j_reply = json.loads(reply.decode('utf-8')[:-1])
        ret = ''
        if 'data' in j_reply:
            if 'trading_day' in j_reply['data']:
                ret = j_reply['data']['trading_day']
        return ret

    def register_switch_day_callback(self, callback):
        self._lock.acquire()
        self._cbs.append(callback)
        self._lock.release()

    def __loop(self):
        while self._started:
            has_new = False
            while True:
                try:
                    reply = self._sub_socket.recv(nnpy.DONTWAIT)
                    j_reply = json.loads(reply.decode('utf-8')[:-1])
                    if 'data' in j_reply:
                        if 'trading_day' in j_reply['data']:
                            self._lock.acquire()
                            self._current = int(j_reply['data']['trading_day'])
                            self._lock.release()
                            has_new = True
                except nnpy.NNError:
                    break
            if has_new:
                self._lock.acquire()
                for cb in self._cbs:
                    cb(self._logger, str(self._current))
                self._lock.release()
            time.sleep(1)
        self._sub_socket.close()
        self._req_socket.close()

    def __get_current_via_req(self):
        j = dict()
        j["req"] = "current"
        j["region"] = Region.CN
        self._req_socket.send(json.dumps(j))
        reply = self._req_socket.recv()
        j_reply = json.loads(reply.decode('utf-8')[:-1])
        if 'data' in j_reply:
            if 'trading_day' in j_reply['data']:
                self._lock.acquire()
                self._current = int(j_reply['data']['trading_day'])
                self._lock.release()


