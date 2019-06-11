import psutil
import functools
import kungfu.command as kfc


@kfc.command(help='journal tools', sensitive=False)
def journal(args, logger):
    if 'journal_command' in args:
        args.name = 'kfc::journal::{}::{}'.format(args.journal_command, psutil.Process().pid)
        COMMANDS[args.journal_command](args, logger)
    else:
        kfc.SUBARGPARSERS['journal'].print_help()


LAST_COMMAND = {}
COMMANDS = {}
SUBARGPARSERS = {}
SUBPARSERS = kfc.SUBARGPARSERS['journal'].add_subparsers(help='journal command help')


def command(help):
    def register_command(func):
        @functools.wraps(func)
        def command_wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        LAST_COMMAND['name'] = func.__name__
        COMMANDS[func.__name__] = command_wrapper
        SUBARGPARSERS[func.__name__] = create_subparser(func.__name__, help=help)
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


def create_subparser(name, help):
    parser = SUBPARSERS.add_parser(name, usage='journal ' + name + ' [<args>]', help=help)
    parser.set_defaults(journal_command=name)
    return parser
