import sys, traceback
from kungfu.log.Logger import create_logger
kfext_logger = create_logger('extensions', 'error')

class ExtensionRegistry:
    def __init__(self):
        self._registry = {}
    def register_extension(self, name, extension):
        self._registry[name] = extension
    def has_extension(self, name):
        return name in self._registry
    def get_extension(self, name):
        return self._registry[name]
    def names(self):
        return self._registry.keys()

EXTENSION_REGISTRY_MD=ExtensionRegistry()
EXTENSION_REGISTRY_TD=ExtensionRegistry()

import pkgutil
__path__ = pkgutil.extend_path(__path__, __name__)
for importer, modname, ispkg in pkgutil.walk_packages(path=__path__, prefix=__name__+'.'):
    try:
        __import__(modname)
        kfext_logger.info('Loaded extension %s', modname)
    except:
        exc_type, exc_obj, exc_tb = sys.exc_info()
        kfext_logger.critical('Bad extension %s, %s %s', modname, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
