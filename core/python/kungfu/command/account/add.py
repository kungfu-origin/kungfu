import click
from kungfu.command.account import *
from PyInquirer import prompt


@account.command()
@click.option('--receive_md', is_flag=True, help='receive market data with this account')
@click.pass_context
def add(ctx, receive_md):
    pass_ctx_from_parent(ctx)
    answers = encrypt(ctx.schema, prompt(make_questions(ctx.schema)))
    account_id = ctx.source + '_' + answers[ctx.schema['key']]
    if find_account(ctx, account_id):
        click.echo('Duplicate account')
    else:
        receive_md = receive_md or not ctx.db.list_source_accounts(ctx.source)
        if receive_md:
            ctx.db.reset_receive_md(ctx.source)
        ctx.db.add_account(account_id=account_id, source_name=ctx.source, receive_md=receive_md, config=answers)
