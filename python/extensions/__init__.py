import os, sys, traceback
import pkgutil
from kungfu.log.Logger import create_logger

kfext_log_level = os.getenv('KF_LOG_LEVEL')
if not kfext_log_level:
    kfext_log_level = 'error'
kfext_logger = create_logger('extensions', kfext_log_level)

class ExtensionRegistry:
    def __init__(self, ext_type):
        self._registry = {}
        self._ext_type = ext_type
    def register_extension(self, name, extension):
        kfext_logger.debug('Registering %s extension %s', self._ext_type, name)
        self._registry[name] = extension
    def has_extension(self, name):
        return name in self._registry
    def get_extension(self, name):
        return self._registry[name]
    def names(self):
        return self._registry.keys()

EXTENSION_REGISTRY_MD=ExtensionRegistry('MD')
EXTENSION_REGISTRY_TD=ExtensionRegistry('TD')
EXTENSIONS={}

extension_path = __path__
__path__ = pkgutil.extend_path(__path__, __name__)
if not os.getenv('KF_NO_EXT'):
    for importer, modname, ispkg in pkgutil.iter_modules(path=__path__, prefix=__name__+'.'):
        try:
            __import__(modname)
            ext_name = modname[len(__name__)+1:]
            extension_path = importer.path
            EXTENSIONS[ext_name] = extension_path
            kfext_logger.info('Loaded extension %s', ext_name)
        except:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            kfext_logger.critical('Bad extension %s, %s %s', modname, exc_type, traceback.format_exception(exc_type, exc_obj, exc_tb))
