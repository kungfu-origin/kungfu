import click
from kungfu.command.account import account, pass_ctx_from_parent


@account.command()
@click.pass_context
def info(ctx):
    pass_ctx_from_parent(ctx)
    click.echo(ctx.db.get_accounts())
