import click
import sys
from kungfu.command.engage import engage, pass_ctx_from_parent


@engage.command(
    help="Format python files with [Black](https://github.com/psf/black)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@click.pass_context
def black(ctx):
    pass_ctx_from_parent(ctx)

    sys.argv = [sys.argv[0], *ctx.args]
    from black.__main__ import patched_main as main

    main()
