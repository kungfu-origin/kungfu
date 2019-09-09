
import kungfu.yijinjing.journal as kfj
import click
from test import test
from test import pass_ctx_from_parent as pass_ctx_from_root
from kungfu.wingchun.ledger_client import LedgerClient
from kungfu.wingchun.constants import LedgerCategory

@test.group()
@click.option('-c', '--category', required=False, default='td', type=click.Choice(kfj.CATEGORIES.keys()), help='category')
@click.option('-g', '--group', required=False, type=str, help='group')
@click.option('-n', '--name', required=False, type=str, help='name')
@click.pass_context
def ledger(ctx, category, group, name):
    pass_ctx_from_root(ctx)
    ctx.category = category
    ctx.group = group
    ctx.name = name
    ctx.ledger_client = LedgerClient(ctx.locator)

def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.category = ctx.parent.category
    ctx.group = ctx.parent.group
    ctx.name = ctx.parent.name
    ctx.ledger_client = ctx.parent.ledger_client

@ledger.command()
@click.option('-i', '--order_id', type=str, help="order id", default="",required=False)
@click.option('--all', is_flag=True, default = False, help='cancel all pending orders')
@click.pass_context
def cancel_order(ctx, order_id, all):
    pass_ctx_from_parent(ctx)
    if order_id:
        print("cancel single order {}".format(order_id))
        result = ctx.ledger_client.cancel_order(order_id)
    elif all and ctx.category == "td":
        account_id = ctx.name
        source_id = ctx.group
        print("cancel all orders for account {}@{}".format(ctx.name, ctx.group))
        result = ctx.ledger_client.cancel_all_order_for_account(source_id, account_id)
    elif all and ctx.category == "strategy":
        print("cancel all orders for strategy {}".format(ctx.name))
        result = ctx.ledger_client.cancel_all_order_for_strategy(ctx.name)
    else:
        print("invalid input")
        result = None
    print("result: {}", result)

@ledger.command()
@click.pass_context
def asset(ctx):
    pass_ctx_from_parent(ctx)
    if ctx.category == "td":
        print("query asset info for account {}@{}".format(ctx.name, ctx.group))
        result = ctx.ledger_client.asset(LedgerCategory.Account, source_id=ctx.group, account_id=ctx.name)
    elif ctx.category == "strategy":
        print("query asset info for strategy {}".format(ctx.name))
        result = ctx.ledger_client.asset(LedgerCategory.Portfolio, client_id=ctx.name)
    else:
        print("invalid input")
        result = None
    print("result: {}", result)

@ledger.command()
@click.pass_context
def calendar(ctx):
    pass_ctx_from_parent(ctx)
    print("result: {}", ctx.ledger_client.calendar())

@ledger.command()
@click.pass_context
def remove_strategy(ctx):
    pass_ctx_from_parent(ctx)
    print("remove strategy {}".format(ctx.name))
    print("result: {}".format(ctx.ledger_client.remove_strategy(ctx.name)))

@ledger.command()
@click.pass_context
def refresh_broker(ctx):
    pass_ctx_from_parent(ctx)
    print("refresh broker state")
    print("result: {}".format(ctx.ledger_client.refresh_broker_state()))

@ledger.command()
@click.option('-d', '--date', type=str,  help="next trading day", required=True)
@click.pass_context
def switch_day(ctx, date):
    pass_ctx_from_parent(ctx)
    print("switch trading day to {}".format(date))
    print("result: {}".format(ctx.ledger_client.switch_trading_day(date)))







