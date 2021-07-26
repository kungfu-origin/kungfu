from . import xtp_demo as ext
from kungfu_extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD

EXTENSION_REGISTRY_MD.register_extension("xtp", ext.MD)
EXTENSION_REGISTRY_TD.register_extension("xtp", ext.TD)
