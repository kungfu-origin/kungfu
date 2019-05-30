import os, sys

if __name__ == '__main__':
    py_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.dirname(py_dir)
    kfc_dir = os.path.join(base_dir, 'build', 'kfc')
    sys.path.append(py_dir)
    sys.path.append(kfc_dir)
    import kungfu.command as kfc
    from kungfu import __main__
