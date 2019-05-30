import os
import json
import numpy
import pyyjj
from kungfu import nanomsg
from kungfu.command import command, arg

@arg('-m', '--message', dest='message', default='{}', type=str, help='message')
@arg('-t', '--times', dest='times', type=int, default=4, help='times')
@command(help='ping kungfu master')
def ping(args, logger):
    base_dir = os.getenv('KF_HOME')

    publisher_socket = create_socket(base_dir, 'publisher', nanomsg.PUSH)
    notice_socket = create_socket(base_dir, 'notice', nanomsg.SUB)
    notice_socket.setsockopt(level=nanomsg.SUB, option=nanomsg.SUB_SUBSCRIBE, value='')
    latency = []
    for t in range(args.times):
        start = pyyjj.now_in_nano()
        sent_bytes = publisher_socket.send(args.message)
        rsp_data = notice_socket.recv()
        end = pyyjj.now_in_nano()
        latency.append(end - start)
        recv_bytes = len(rsp_data)
        pong_messsage = rsp_data
        print('[{}] {}/{} bytes time={} ns'.format(t + 1, sent_bytes, recv_bytes, latency[-1]))
        logger.debug('ping: %s, pong: %s', args.message, pong_messsage)
    print('round-trip min/avg/max/stddev = {:.0f}/{:.0f}/{:.0f}/{:.0f} ns'.format(
        numpy.min(latency),numpy.mean(latency),numpy.max(latency),numpy.std(latency)
    ))
    publisher_socket.close()
    notice_socket.close()

def create_socket(base_dir, name, nn_mode):
    socket_path = os.path.join(base_dir, 'socket', 'system', name + '.sock')
    socket_addr = 'ipc://' + socket_path
    socket = pyyjj.socket(nanomsg.AF_SP, nn_mode)
    socket.connect(socket_addr)
    return socket
