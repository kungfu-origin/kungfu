import click
import glob
import os
from collections import deque
from kungfu.command.archive import archive, pass_ctx_from_parent


@archive.command()
@click.pass_context
def list(ctx):
    pass_ctx_from_parent(ctx)
    deque(map(print_archive, glob.glob(os.path.join(ctx.archive_dir, "*.zip"))))


def print_archive(archive_file):
    archive_name = os.path.basename(archive_file)
    print(archive_name[4:-4])
