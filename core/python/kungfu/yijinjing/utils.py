import glob
import os
from collections import deque

def prune_layout_files(base_dir, layout, mode):
    search_path = os.path.join(base_dir, '*', '*', '*', layout, mode, '*')
    deque(map(os.remove, filter(os.path.isfile, glob.glob(search_path))))
