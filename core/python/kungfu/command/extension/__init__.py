import os
import click
from kungfu.command import kfc


@kfc.group()
@click.pass_context
def extension(ctx):
    ctx.logger = ctx.parent.logger
    if not os.getenv('KF_NO_EXT'):
        pass
    else:
        print('Extension disabled by KF_NO_EXT')
        ctx.logger.warning('Trying to manage extension while disallowed by KF_NO_EXT')
