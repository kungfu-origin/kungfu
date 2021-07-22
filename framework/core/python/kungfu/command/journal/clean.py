import click
import glob
import os
import zipfile
from kungfu.yijinjing import time as kft
from kungfu.command.journal import journal, pass_ctx_from_parent


@journal.command()
@click.option("-A", "--archive", is_flag=True, help="archive")
@click.option("-D", "--dry_run", is_flag=True, help="dry run")
@click.pass_context
def clean(ctx, archive, dry_run):
    pass_ctx_from_parent(ctx)
    search_path = os.path.join(ctx.home, "*", "*", "*", "journal", "*", "*.journal")
    journal_files = glob.glob(search_path)
    if dry_run:
        for journal_file in journal_files:
            click.echo(f"rm {journal_file}")
        return
    if archive:
        datestr = kft.strfnow("%Y%m%d-%H%M%S")
        archive_path = os.path.join(ctx.home, f"KFA-{datestr}.zip")
        archive_zip = zipfile.ZipFile(
            archive_path, mode="w", compression=zipfile.ZIP_LZMA
        )
        for journal_file in journal_files:
            archive_zip.write(journal_file)
        click.echo(f"archived to {archive_path}")
    for journal_file in journal_files:
        os.remove(journal_file)
    click.echo(f"cleaned {len(journal_files)} journal files")
