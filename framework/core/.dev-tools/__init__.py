import os
import sys

from os.path import abspath, dirname

base_dir = dirname(abspath(dirname(__file__)))
python_dir = os.path.join(base_dir, "python")
kfc_dir = os.path.join(base_dir, "dist", "kfc")

sys.path.append(python_dir)
sys.path.append(kfc_dir)
os.environ["PATH"] += os.pathsep + kfc_dir


__built__ = os.path.exists(kfc_dir)
