import pyyjj
import json
import numpy
import click
from kungfu.command import kfc, pass_ctx_from_parent


@kfc.command()
@click.option('-t', '--times', type=int, default=4, help='times')
@click.option('-m', '--message', type=str, help='message')
@click.pass_context
def ping(ctx, times, message):
    pass_ctx_from_parent(ctx)
    pyyjj.setup_log(ctx.name)
    io_device = pyyjj.create_io_device_client(ctx.parent.name)
    latency = []
    for t in range(times):
        start = pyyjj.now_in_nano()
        request = {'messsage': message} if message else {}
        sent_bytes = io_device.publisher.publish(json.dumps(request))
        while not io_device.observer.wait():
            pass
        rsp_data = io_device.observer.get_notice()
        end = pyyjj.now_in_nano()
        latency.append(end - start)
        recv_bytes = len(rsp_data)
        print('[{}] {}/{} bytes time={} ns'.format(t + 1, sent_bytes, recv_bytes, latency[-1]))
    click.echo('round-trip min/avg/max/stddev = {:.0f}/{:.0f}/{:.0f}/{:.0f} ns'.format(
        numpy.min(latency),numpy.mean(latency),numpy.max(latency),numpy.std(latency)
    ))
