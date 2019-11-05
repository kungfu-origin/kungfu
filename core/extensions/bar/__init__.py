from . import ${PROJECT_NAME} as ext
from extensions import EXTENSION_REGISTRY_MD
EXTENSION_REGISTRY_MD.register_extension('bar', ext.MD)
