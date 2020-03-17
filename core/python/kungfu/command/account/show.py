import re
import json
import click
from pykungfu import longfist
from pykungfu import yijinjing as yjj
from kungfu.command.account import account, pass_ctx_from_parent
from tabulate import tabulate


@account.command()
@click.pass_context
def show(ctx):
    pass_ctx_from_parent(ctx)
    accounts = ctx.profile.get_all(longfist.types.Config())
    headers = [obj['key'] for obj in ctx.schema['td_config']]
    data = [sort_account_config(json.loads(account_data.value), headers) for account_data in accounts if account_data.category == yjj.category.TD]
    table = tabulate(data, headers, tablefmt='simple')
    click.echo(table)


def sort_account_config(account_config, headers):
    return [account_config[header] if header != 'password' else re.sub(r'.', '*', account_config[header]) for header in headers]
