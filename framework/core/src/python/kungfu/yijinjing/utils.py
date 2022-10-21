import glob
import os
from collections import deque


def prune_layout_files(base_dir, layout, mode):
    search_path = os.path.join(base_dir, "*", "*", "*", layout, mode, "*")
    for file in filter(os.path.isfile, glob.glob(search_path)):
        try:
            os.remove(file)
        except:
            continue

def prue_layout_dirs_before_timestamp(base_dir, layout, mode, timestamp):
    search_path = os.path.join(base_dir, "*", "*", "*", layout, mode, "*")
    for file in filter(os.path.isfile, glob.glob(search_path)):
        created_at = os.path.getctime(file)
        modify_at = os.path.getmtime(file)
        if created_at < timestamp or modify_at < timestamp:
            try:
                os.remove(file)
            except:
                continue