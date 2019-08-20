import re
import click
from kungfu.command.account import account, pass_ctx_from_parent
from tabulate import tabulate


@account.command()
@click.pass_context
def show(ctx):
    pass_ctx_from_parent(ctx)
    accounts = ctx.db.list_source_accounts(ctx.source)
    headers = [obj['key'] for obj in ctx.schema['config']]
    data = [[account_data['receive_md']] + sort_account_config(account_data['config'], headers) for account_data in accounts]
    table = tabulate(data, headers=['receive_md'] + headers, tablefmt='simple')
    click.echo(table)


def sort_account_config(account_config, headers):
    return [account_config[header] if header != 'password' else re.sub(r'.', '*', account_config[header]) for header in headers]
