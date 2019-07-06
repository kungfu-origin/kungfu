import os
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root


@kfc.group()
@click.pass_context
def extension(ctx):
    pass_ctx_from_root(ctx)
    if not os.getenv('KF_NO_EXT'):
        pass
    else:
        print('Extension disabled by KF_NO_EXT')
        ctx.logger.warning('Trying to manage extension while disallowed by KF_NO_EXT')
