from . import ${PROJECT_NAME} as ext
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD
EXTENSION_REGISTRY_MD.register_extension('ctp', ext.MD)
EXTENSION_REGISTRY_TD.register_extension('ctp', ext.TD)