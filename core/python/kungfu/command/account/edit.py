import click
from kungfu.command.account import account, pass_ctx_from_parent, make_questions, encrypt
from PyInquirer import prompt


@account.command()
@click.pass_context
def edit(ctx):
    pass_ctx_from_parent(ctx)
    answers = encrypt(prompt(make_questions(ctx.schema)))
    click.echo(answers)
