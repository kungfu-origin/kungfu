from pykungfu import yijinjing as pyyjj
import click
from kungfu.command import kfc, pass_ctx_from_parent
from extensions import EXTENSION_REGISTRY_TD
from kungfu.data.sqlite.data_proxy import AccountsDB


@kfc.command(help_priority=3)
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_TD.names()), help='destination to send order')
@click.option('-a', '--account', type=str, help='account')
@click.option('-x', '--low_latency', is_flag=True, help='run in low latency mode')
@click.pass_context
def td(ctx, source, account, low_latency):
    pass_ctx_from_parent(ctx)
    ctx.db = AccountsDB(pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator), 'accounts')
    td_config = ctx.db.get_td_account_config(source, source + '_' + account)

    print(td_config, '-----')
    ext = EXTENSION_REGISTRY_TD.get_extension(source)(low_latency, ctx.locator, account, td_config)
    ext.run()
