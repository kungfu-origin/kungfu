import os
import json
import numpy
import nnpy

import pyyjj

class Ping:
    def __init__(self, logger):
        self.logger = logger

        base_dir = os.getenv('KF_HOME')

        self._emitter_socket = self.create_socket(base_dir, 'emitter', nnpy.PUSH)
        self._notice_socket = self.create_socket(base_dir, 'notice', nnpy.SUB)
        self._notice_socket.setsockopt(level=nnpy.SUB, option=nnpy.SUB_SUBSCRIBE, value='')

    def create_socket(self, base_dir, name, nn_mode):
        socket_path = os.path.join(base_dir, 'socket', name + '.sock')
        socket_addr = 'ipc://' + socket_path
        socket = nnpy.Socket(nnpy.AF_SP, nn_mode)
        rc = socket.connect(socket_addr)
        return socket

    def ping(self, ping_message, times):
        latency = []
        for t in range(times):
            start = pyyjj.nano_time()
            sent_bytes = self._emitter_socket.send(ping_message)
            rsp_data = self._notice_socket.recv()
            end = pyyjj.nano_time()
            latency.append(end - start)
            recv_bytes = len(rsp_data)
            pong_messsage = rsp_data.decode('utf-8')
            print('[{}] {}/{} bytes time={} ns'.format(t + 1, sent_bytes, recv_bytes, latency[-1]))
            self.logger.debug('ping: %s, pong: %s', ping_message, pong_messsage)
        print('round-trip min/avg/max/stddev = {:.0f}/{:.0f}/{:.0f}/{:.0f} ns'.format(
            numpy.min(latency),numpy.mean(latency),numpy.max(latency),numpy.std(latency)
            ))

    def stop(self):
        self._emitter_socket.close()
        self._notice_socket.close()
