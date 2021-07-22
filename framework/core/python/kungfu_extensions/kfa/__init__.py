from kungfu_extensions import EXTENSION_REGISTRY_DATA
from .adapter import KfaAdapter

EXTENSION_REGISTRY_DATA.register_extension("kfa", KfaAdapter)
