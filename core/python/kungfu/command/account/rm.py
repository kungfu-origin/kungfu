import click
from kungfu.command.account import *


@account.command()
@click.option('-i', '--id', type=str, required=True, help='id')
@click.pass_context
def rm(ctx, id):
    pass_ctx_from_parent(ctx)
    account_id = ctx.source + '_' + id
    ctx.db.delete_account(account_id)
