import click
import kungfu.yijinjing.journal as kfj
from kungfu.command.journal import journal, pass_ctx_from_parent


@journal.command()
@click.option("-i", "--session_id", type=int, required=True, help="session id")
@click.option(
    "-t",
    "--io_type",
    type=click.Choice(["all", "in", "out"]),
    default="all",
    help="input or output during this session",
)
@click.pass_context
def trace(ctx, session_id, io_type):
    pass_ctx_from_parent(ctx)
    kfj.trace_journal(ctx, session_id, io_type)
