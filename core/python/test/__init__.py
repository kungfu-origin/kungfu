import click
import extensions


@click.group(invoke_without_command=True)
@click.pass_context
def test(ctx):
    if ctx.invoked_subcommand is None:
        click.echo(test.get_help(ctx))
    pass

