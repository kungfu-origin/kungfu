from kungfu_extensions import EXTENSION_REGISTRY_DATA
from .adapter import TushareAdapter

EXTENSION_REGISTRY_DATA.register_extension('tushare', TushareAdapter)
