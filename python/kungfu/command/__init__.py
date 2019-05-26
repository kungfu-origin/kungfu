import argparse
import os, sys, platform, atexit
import functools

from kungfu.log.Logger import create_logger

parser = argparse.ArgumentParser(description='Kung Fu Command', usage='kfc [<options>] <command> [<args>]')
parser.add_argument('-V', '--py-version', dest='py_version', action="store_true", help='python version')
parser.add_argument('-f', '--home', dest='home', help='kungfu home folder')
parser.add_argument('-l', '--log-level', dest='log_level', type=str, choices=['trace','debug','info','warning','error','critical'], help='logging level')
subparsers = parser.add_subparsers(help='sub-command help')

LAST_COMMAND = {}
COMMANDS = {}
SUBARGPARSERS = {}

def command(help, sensitive=True):
    def register_command(func):
        @functools.wraps(func)
        def command_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        LAST_COMMAND['name'] = func.__name__
        COMMANDS[func.__name__] = command_wrapper
        SUBARGPARSERS[func.__name__] = create_subparser(func.__name__, help=help, sensitive=sensitive)
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

def create_subparser(name, help, sensitive):
    parser = subparsers.add_parser(name, usage='kfc ' + name + ' [<args>]', help=help)
    parser.set_defaults(command=name)
    if sensitive:
        parser.add_argument('-n', '--name', dest='name', help='name for the process')
        parser.add_argument('-x', '--low-latency', dest='low_latency', action = "store_true", help='run in low latency mode')
    return parser

def execute():
    args = parser.parse_args()
    if args.py_version:
        print('Python', platform.python_version())
        return
    if 'command' not in args:
        parser.print_help()
        return

    # set KF_HOME
    if args.home:
        os.environ['KF_HOME'] = args.home
    else:
        args.home = os.environ['KF_HOME']

    # set log level according to environment variable KF_LOG_LEVEL and arg -l/--log-level
    env_log_level = os.getenv('KF_LOG_LEVEL')
    if not env_log_level and not args.log_level:
        args.log_level = 'warning'
    if env_log_level and not args.log_level:
        args.log_level = os.environ['KF_LOG_LEVEL']
    os.environ['KF_LOG_LEVEL'] = args.log_level
    logger = create_logger(args.name if 'name' in args else args.command, args.log_level)

    # use dispatch pattern to invoke method with same name
    COMMANDS[args.command](args, logger)
