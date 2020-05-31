from kungfu_extensions import EXTENSION_REGISTRY_DATA
from .adapter import RicequantAdapter

EXTENSION_REGISTRY_DATA.register_extension('ricequant', RicequantAdapter)
