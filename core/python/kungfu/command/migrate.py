import click
import glob
import os
import shutil
from kungfu.yijinjing import LAYOUT_LOCATION_PATTERN
from kungfu.command import kfc, pass_ctx_from_parent
from pykungfu import yijinjing as yjj


@kfc.command()
@click.pass_context
def migrate(ctx):
    pass_ctx_from_parent(ctx)
    migrate_to_2_3(ctx)


def migrate_to_2_3(ctx):
    if len(os.listdir(ctx.runtime_dir)) != 0:
        print(f'runtime {ctx.runtime_dir} is not empty')

    for category_value in yjj.category.__members__:
        category_name = yjj.get_category_name(yjj.category.__members__[category_value])
        category_dir = os.path.join(ctx.home, category_name)
        if os.path.exists(category_dir):
            shutil.move(category_dir, ctx.runtime_dir)

    search_path = os.path.join(ctx.runtime_dir, '*', '*', '*', '*', '*', '*.*')
    for file in glob.glob(search_path):
        match = LAYOUT_LOCATION_PATTERN.match(file[len(ctx.runtime_dir) + 1:])
        if match:
            category = match.group(1)
            group = match.group(2)
            name = match.group(3)
            layout = match.group(4)
            mode = match.group(5)
            filename = match.group(6)
