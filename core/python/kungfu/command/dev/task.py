import pyyjj
import json
import click
from kungfu.command.dev import dev, pass_ctx_from_parent
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD
from kungfu.log import create_logger

from kungfu.data.sqlite.data_proxy import make_url, DataProxy


@dev.command()
@click.option('-t', '--gateway_type', required=True, type=click.Choice(['md', 'td']), help='gateway type')
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='gateway source')
@click.option('-a', '--account', type=str, help='account')
@click.option('-c', '--config', type=str, help='set config')
@click.pass_context
def task(ctx, gateway_type, source, account, config):
    pass_ctx_from_parent(ctx)
    db = DataProxy(make_url(ctx.locator, ctx.system_config_location, "task"))
    config_name = gateway_type + '_' + source + ('' if gateway_type == 'md' else '_' + account)
    if config:
        db.set_task_config(config_name, json.loads(config))
    record = db.get_task_config(config_name)
    click.echo(config_name)
    click.echo(record)
