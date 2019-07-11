import pyyjj
import json
import click
from kungfu.command.test import test, pass_ctx_from_parent, send_to_passive


@test.command()
@click.pass_context
def account(ctx):
    pass_ctx_from_parent(ctx)
    click.echo('account test')
    td_location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.TD, 'passive', 'test', ctx.locator)
    td_passive_rep_sock = ctx.io_device.connect_socket(td_location, pyyjj.protocol.REQUEST)
    send_to_passive(ctx, td_passive_rep_sock, td_location, {
        'msg_type': 401,
        'state': 3
    })
