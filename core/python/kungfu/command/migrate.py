import click
from kungfu.yijinjing import migrate_to_2_3
from kungfu.command import kfc, pass_ctx_from_parent


@kfc.command()
@click.pass_context
def migrate(ctx):
    pass_ctx_from_parent(ctx)
    migrate_to_2_3(ctx)
