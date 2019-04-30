
from kungfu.wingchun.TaskScheduler import TaskScheduler
from kungfu.wingchun.constants import MsgType
import nnpy, json, pyyjj
import sys
import signal

class EventLoop:
    def __init__(self, name):
        self._quit = False
        self._name = name
        self._reader = None
        self._scheduler = TaskScheduler()
        self._sockets = []
        self._quote_cb = None
        self._entrust_cb = None
        self._trans_cb = None
        self._login_cb = None
        self._sub_cb = None
        self._input_cb = None
        self._action_cb = None
        self._order_cb = None
        self._trade_cb = None
        self._algo_input_cb = None
        self._signal_received = -1
        self._signal_callbacks = []

    def subscribe_nanomsg(self, url):
        socket = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
        socket.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, '')
        socket.connect(url)
        self._sockets.append(socket)

    def bind_nanomsg(self, url):
        socket = nnpy.Socket(nnpy.AF_SP, nnpy.REP)
        socket.bind(url)
        self._sockets.append(socket)

    def add_socket(self, socket):
        self._sockets.append(socket)

    def subscribe_yjj_journal(self, journal_folder, journal_name, offset_nano):
        if self._reader is None:
            self._reader = pyyjj.createReader([journal_folder], [journal_name], offset_nano, self._name)
        else:
            idx = self._reader.addJ(journal_folder, journal_name)
            self._reader.seekJ(idx, offset_nano)

    def get_nano(self):
        return self._scheduler.get_nano()

    def register_nanotime_callback(self, nano, callback):
        self._scheduler.insert_callback_at(nano, callback)

    def register_nanotime_callback_at_next(self, time_str, callback):
        self._scheduler.insert_callback_at_next(time_str, callback)

    def register_quote_callback(self, callback):
        self._quote_cb = callback

    def register_entrust_callback(self, callback):
        self._entrust_cb = callback

    def register_transaction_callback(self, callback):
        self._trans_cb = callback

    def register_login_callback(self, callback):
        self._login_cb = callback

    def register_subscribe_callback(self, callback):
        self._sub_cb = callback

    def register_order_input_callback(self, callback):
        self._input_cb = callback

    def register_order_action_callback(self, callback):
        self._action_cb = callback

    def register_order_callback(self, callback):
        self._order_cb = callback

    def register_trade_callback(self, callback):
        self._trade_cb = callback

    def register_algo_order_input_callback(self, callback):
        self._algo_input_cb = callback

    def register_signal_callback(self, callback):
        self._signal_callbacks.append(callback)

    def run(self):
        signal.signal(signal.SIGINT, self.__handle_signal)
        signal.signal(signal.SIGTERM, self.__handle_signal)
        signal.signal(signal.SIGHUP, self.__handle_signal)
        signal.signal(signal.SIGQUIT, self.__handle_signal)

        self._quit = False
        while not self._quit and self._signal_received < 0:
            self.__iteration()
        if self._signal_received >=0 :
            print("signal received ", self._signal_received)
            for cb in self._signal_callbacks:
                cb(self._signal_received)

    def stop(self):
        self._quit = True

    def __iteration(self):
        nano = -1
        if self._reader is not None:
            frame = self._reader.next()
            if frame is not None:
                nano = frame.nano()
                self.__handle_frame(frame)
        if nano == -1:
            nano = pyyjj.nano()
        self._scheduler.update_nano(nano)
        for socket in self._sockets:
            try:
                reply = socket.recv(nnpy.DONTWAIT)
                j_reply = json.loads(reply.decode('utf-8')[:-1])
                msg_type = j_reply["msg_type"]
                recipient = j_reply["recipient"]
                sender = j_reply["sender"]
                data = j_reply["data"]
                self.__handle_nanomsg(msg_type, recipient, sender, data)
            except nnpy.NNError as nnpy_e:
                print(nnpy_e)
            except Exception as e:
                print(e)

    def __handle_frame(self, frame):
        msg_type = frame.msg_type()
        if msg_type == MsgType.Quote:
            if self._quote_cb is not None:
                self._quote_cb(frame.get_data())
        elif msg_type == MsgType.Entrust:
            if self._entrust_cb is not None:
                self._entrust_cb(frame.get_data())
        elif msg_type == MsgType.Transaction:
            if self._trans_cb is not None:
                self._trans_cb(frame.get_data())
        elif msg_type == MsgType.OrderInput:
            if self._input_cb is not None:
                self._input_cb(frame.get_data())
        elif msg_type == MsgType.OrderAction:
            if self._action_cb is not None:
                self._action_cb(frame.get_data())
        elif msg_type == MsgType.Order:
            if self._order_cb is not None:
                self._order_cb(frame.get_data())
        elif msg_type == MsgType.Trade:
            if self._trade_cb is not None:
                self._trade_cb(frame.get_data())
        elif msg_type == MsgType.AlgoOrderInput:
            if self._algo_input_cb is not None:
                try:
                    j = json.loads(frame.get_data())
                    self._algo_input_cb(j['order_id'], j['algo_type'], j['client_id'], j['algo_order_input'])
                except Exception as e:
                    print(e)
        else:
            pass

    def __handle_nanomsg(self, msg_type, recipient, sender, data):
        if msg_type == MsgType.ReqLogin:
            if self._login_cb is not None:
                self._login_cb(recipient, sender)
        elif msg_type == MsgType.Subscribe:
            if self._sub_cb is not None:
                self._sub_cb(recipient, data)
        else:
            pass

    def __handle_signal(self, signal, frame):
        self._signal_received = signal