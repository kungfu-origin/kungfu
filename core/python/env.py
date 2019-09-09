
import os
import sys

def setup_environment_variables():
    if not getattr(sys, 'frozen', False):
        py_dir = os.path.dirname(os.path.abspath(__file__))
        base_dir = os.path.dirname(py_dir)
        kfc_dir = os.path.join(base_dir, 'build', 'kfc')
        sys.path.append(py_dir)
        sys.path.append(kfc_dir)
        os.environ['PATH'] = kfc_dir + os.pathsep + os.environ['PATH']