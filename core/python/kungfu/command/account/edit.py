import click
from kungfu.command.account import account, pass_ctx_from_parent, make_questions, encrypt
from PyInquirer import prompt


@account.command()
@click.option('--receive_md', is_flag=True, help='receive market data with this account')
@click.option('-i', '--id', type=str, required=True, help='id')
@click.pass_context
def edit(ctx, receive_md, id):
    pass_ctx_from_parent(ctx)
    account_id = ctx.source + '_' + id
    account_data = ctx.db.find_account(account_id)
    if account_data:
        answers = encrypt(ctx.schema, prompt(make_questions(ctx.schema, account_data['config'])))
        if receive_md:
            ctx.db.reset_receive_md(ctx.source)
        receive_md = receive_md or account_data['receive_md']
        ctx.db.add_account(account_id=account_id, source_name=ctx.source, receive_md=receive_md, config=answers)
    else:
        click.echo('Account not found')
