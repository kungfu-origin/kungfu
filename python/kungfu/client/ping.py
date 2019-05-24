import os
import json
import numpy
import nnpy, pyyjj
from kungfu.command import command, arg

@arg('-m', '--message', dest='message', default='{}', type=str, help='message')
@arg('-t', '--times', dest='times', type=int, default=4, help='times')
@command(help='ping kungfu master')
def ping(args, logger):
    base_dir = os.getenv('KF_HOME')

    emitter_socket = create_socket(base_dir, 'emitter', nnpy.PUSH)
    notice_socket = create_socket(base_dir, 'notice', nnpy.SUB)
    notice_socket.setsockopt(level=nnpy.SUB, option=nnpy.SUB_SUBSCRIBE, value='')
    latency = []
    for t in range(args.times):
        start = pyyjj.nano_time()
        sent_bytes = emitter_socket.send(args.message)
        rsp_data = notice_socket.recv()
        end = pyyjj.nano_time()
        latency.append(end - start)
        recv_bytes = len(rsp_data)
        pong_messsage = rsp_data.decode('utf-8')
        print('[{}] {}/{} bytes time={} ns'.format(t + 1, sent_bytes, recv_bytes, latency[-1]))
        logger.debug('ping: %s, pong: %s', args.message, pong_messsage)
    print('round-trip min/avg/max/stddev = {:.0f}/{:.0f}/{:.0f}/{:.0f} ns'.format(
        numpy.min(latency),numpy.mean(latency),numpy.max(latency),numpy.std(latency)
    ))
    emitter_socket.close()
    notice_socket.close()

def create_socket(base_dir, name, nn_mode):
    socket_path = os.path.join(base_dir, 'socket', name + '.sock')
    socket_addr = 'ipc://' + socket_path
    socket = nnpy.Socket(nnpy.AF_SP, nn_mode)
    rc = socket.connect(socket_addr)
    return socket
