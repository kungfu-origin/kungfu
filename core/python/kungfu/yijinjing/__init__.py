import os
import re
import glob
import shutil
from pykungfu import longfist as lf
from pykungfu import yijinjing as yjj
from . import journal as kfj

os_sep = re.escape(os.sep)

LAYOUT_LOCATION_REGEX = '{}{}{}{}{}{}{}{}{}{}{}'.format(
    r'(.*)', os_sep,  # category
    r'(.*)', os_sep,  # group
    r'(.*)', os_sep,  # name
    r'(.*)', os_sep,  # layout
    r'(.*)', os_sep,  # mode
    r'(.*)'           # filename
)
LAYOUT_LOCATION_PATTERN = re.compile(LAYOUT_LOCATION_REGEX)


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
