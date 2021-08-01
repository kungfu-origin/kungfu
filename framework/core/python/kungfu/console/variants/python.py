import click
import importlib
import platform
import os
import sys


@click.command(
    context_settings=dict(
        allow_extra_args=True,
        allow_interspersed_args=True,
        ignore_unknown_options=True,
    )
)
@click.option(
    "-E",
    "ignore_python_environment",
    is_flag=True,
    help="name for the process, defaults to command if not set",
)
@click.option(
    "-c",
    "code",
    type=str,
    help="program passed in as string (terminates option list)",
)
@click.option(
    "-m",
    "module",
    type=str,
    help="run library module as a script (terminates option list)",
)
@click.option(
    "-s",
    "ignore_user_site",
    is_flag=True,
    help="don't add user site directory to sys.path; also PYTHONNOUSERSITE",
)
@click.option(
    "-S",
    "ignore_import_site",
    is_flag=True,
    help="don't imply 'import site' on initialization",
)
@click.help_option("-h", "--help")
@click.version_option(
    platform.python_version(),
    "-V",
    "--version",
    message=f"Python {platform.python_version()}",
)
@click.argument("argv", type=str, nargs=-1)
def main(argv, **options):
    __name__ = "__main__"

    if options["code"]:
        exec(options["code"])
        return

    if options["module"]:
        sys.argv = [sys.executable, *argv]
        run_module(options["module"])
        return

    if argv:
        sys.argv = [sys.executable, *argv[1:]]
        __file__ = str(os.path.abspath(argv[0]))
        dist_root = str(os.path.dirname(sys.executable))
        if __file__.startswith(dist_root):
            module_name = (
                __file__.replace(dist_root, "")
                .replace(".py", "")
                .replace(os.sep, ".")[1:]
            )
            run_module(module_name, True)
            return
        with open(__file__, "r") as source:
            code = "".join(source.readlines())
        exec(code)
        return

    pass


def run_module(module_name, is_single_file=False):
    main_name = "__main__"
    main_module = (
        importlib.import_module(f"{module_name}.{main_name}")
        if not is_single_file
        else importlib.import_module(module_name)
    )
    main_loader = main_module.__loader__
    main_loader.name = main_module.__name__ = main_name
    main_loader.exec_module(main_module)
