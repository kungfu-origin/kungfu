import sys
import psutil
import json


def checkin(ctx, io_device):
    pid = psutil.Process().pid
    request = {
        "request": "checkin",
        "name": ctx.name,
        "pid": pid
    }
    request_str = json.dumps(request)
    ctx.logger.info('checking int: %s', request_str)
    response_str = io_device.service.request(request_str)
    response = json.loads(response_str)
    if 'success' not in response or not response['success']:
        ctx.logger.critical('Unable to checkin with master')
        sys.exit(-1)
    else:
        ctx.logger.info('%s checked in with master', ctx.name)


def checkout(ctx, io_device):
    request = {
        "request": "checkout",
        "name": ctx.name,
        "pid": psutil.Process().pid
    }
    request_str = json.dumps(request)
    ctx.logger.info('checking out: %s', request_str)
    response_str = io_device.service.request(request_str)
    response = json.loads(response_str)
    if 'success' not in response or not response['success']:
        ctx.logger.critical('Unable to checkout with master')
        sys.exit(-1)
    else:
        ctx.logger.info('%s checked out with master', ctx.name)

