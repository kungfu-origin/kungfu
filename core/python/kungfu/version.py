
import os
import datetime
import subprocess

VERSION = (2, 1, 0, 'final', 0)


def get_version():
    main = get_main_version(VERSION)
    sub = ''
    if VERSION[3] == 'alpha' and VERSION[4] == 0:
        git_changeset = get_git_changeset()
        if git_changeset:
            sub = '.dev%s' % git_changeset
    elif VERSION[3] != 'final':
        mapping = {'alpha': 'a', 'beta': 'b', 'rc': 'rc'}
        sub = mapping[VERSION[3]] + str(VERSION[4])
    return main + sub


def get_main_version(version):
    parts = 2 if version[2] == 0 else 3
    return '.'.join(str(x) for x in version[:parts])


def get_git_changeset():
    repo_dir = os.path.dirname(os.path.dirname(os.path.realpath('__file__')))
    git_log = subprocess.run(
        ['git', 'log', '--pretty=format:%ct', '--quiet', '-1', 'HEAD'],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        cwd=repo_dir, universal_newlines=True,
    )
    timestamp = git_log.stdout
    try:
        timestamp = datetime.datetime.utcfromtimestamp(int(timestamp))
    except ValueError:
        return None
    return timestamp.strftime('%Y%m%d%H%M%S')
