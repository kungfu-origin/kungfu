import argparse
import atexit
import functools
import os
import platform

import daemon

from kungfu.log import create_logger

ROOT_PARSER = argparse.ArgumentParser(description='Kung Fu Core', usage='kfc [<options>] <command> [<args>]')
ROOT_PARSER.add_argument('-V', '--py-version', dest='py_version', action="store_true", help='python version')
ROOT_PARSER.add_argument('-f', '--home', dest='home', type=str, help='kungfu home folder')
ROOT_PARSER.add_argument('-l', '--log-level', dest='log_level', type=str, choices=['trace','debug','info','warning','error','critical'], default='warning', help='logging level')
SUBPARSERS = ROOT_PARSER.add_subparsers(help='sub-command help')

LAST_COMMAND = {}
COMMANDS = {}
SUBARGPARSERS = {}

DAEMON_CONTEXT = daemon.DaemonContext()


def command(help, sensitive=True, enable_daemonize=False):
    def register_command(func):
        @functools.wraps(func)
        def command_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        LAST_COMMAND['name'] = func.__name__
        COMMANDS[func.__name__] = command_wrapper
        SUBARGPARSERS[func.__name__] = create_subparser(func.__name__, help=help, sensitive=sensitive, enable_daemonize=enable_daemonize)
        return command
    return register_command


def arg(*arg_args, **arg_kwargs):
    def register_arg(func):
        @functools.wraps(func)
        def arg_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        SUBARGPARSERS[LAST_COMMAND['name']].add_argument(*arg_args, **arg_kwargs)
        return arg
    return register_arg


def exit_handler(task):
    task.stop()


def create_subparser(name, help, sensitive, enable_daemonize):
    parser = SUBPARSERS.add_parser(name, usage='kfc ' + name + ' [<args>]', help=help)
    parser.set_defaults(command=name)
    if sensitive:
        parser.add_argument('-n', '--name', dest='name', type=str, help='name for the process, defaults to command if not set')
        parser.add_argument('-x', '--low-latency', dest='low_latency', action="store_true", help='run in low latency mode')
    if enable_daemonize:
        parser.add_argument('-z', '--daemon', dest='daemon', action="store_true", help='run as daemon')
    return parser


def execute():
    args = ROOT_PARSER.parse_args()
    if args.py_version:
        print('Python', platform.python_version())
        return
    if 'command' not in args:
        ROOT_PARSER.print_help()
        return
    if 'name' not in args or not args.name:
        args.name = args.command

    # set KF_HOME
    if args.home:
        os.environ['KF_HOME'] = args.home
    else:
        args.home = os.environ['KF_HOME']

    # set log level according to environment variable KF_LOG_LEVEL and arg -l/--log-level
    env_log_level = os.getenv('KF_LOG_LEVEL')
    if env_log_level:
        args.log_level = env_log_level
    os.environ['KF_LOG_LEVEL'] = args.log_level
    logger = create_logger(args.name, args.log_level)

    if 'daemon' in args and args.daemon:
        logger.info('kfc %s becoming daemon', args.command)
        atexit.register(lambda args: logger.info('kfc daemon %s exit', args.name), args)
        DAEMON_CONTEXT.open()
        logger.info('kfc %s become daemon', args.command)

    # use dispatch pattern to invoke method with same name
    COMMANDS[args.command](args, logger)
