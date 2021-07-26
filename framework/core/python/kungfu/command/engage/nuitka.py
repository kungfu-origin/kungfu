import click
import sys
from kungfu.command.engage import engage, pass_ctx_from_parent


@engage.command(
    help="Compile and bundle python files with [Nuitka](https://nuitka.net)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@click.pass_context
def nuitka(ctx):
    pass_ctx_from_parent(ctx)

    sys.argv = [sys.argv[0], *ctx.args]
    from nuitka import Options, MainControl

    Options.parseArgs(will_reexec=False)
    Options.commentArgs()
    MainControl.main()
