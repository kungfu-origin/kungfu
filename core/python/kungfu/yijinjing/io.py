import sys
import psutil
import json


def checkin(args, logger, io_device):
    pid = psutil.Process().pid
    request = {
        "request": "checkin",
        "name": args.name,
        "pid": pid
    }
    request_str = json.dumps(request)
    response_str = io_device._service.request(request_str)
    response = json.loads(response_str)
    if 'success' not in response or not response['success']:
        logger.critical('Unable to checkin with master')
        sys.exit(-1)
    else:
        logger.info('%s checked in with master', args.name)


def checkout(args, logger, io_device):
    request = {
        "request": "checkout",
        "name": args.name,
        "pid": psutil.Process().pid
    }
    request_str = json.dumps(request)
    response_str = io_device._service.request(request_str)
    response = json.loads(response_str)
    if 'success' not in response or not response['success']:
        logger.critical('Unable to checkout with master')
        sys.exit(-1)
    else:
        logger.info('%s checked out with master', args.name)

