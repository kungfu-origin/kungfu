import os
import sys
import platform


def default_sdk_home():
    osname = platform.system()
    if osname == 'Linux':
        return "/opt/Kungfu/resources/kfc"
    elif osname == 'Darwin':
        return "/Applications/Kungfu.app/Contents/Resources/kfc"
    elif osname == 'Windows':
        return "C:\\Program Files\\Kungfu\\resources\\kfc"


def setup_environment_variables():
    if not getattr(sys, 'frozen', False):
        if not hasattr(sys, 'kf_sdk_home'):
            sys.kf_sdk_home = default_sdk_home()
        sys.path.append(sys.kf_sdk_home)
        os.environ['PATH'] = sys.kf_sdk_home + os.pathsep + os.environ['PATH']