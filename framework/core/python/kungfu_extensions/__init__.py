import kungfu
import os

from kungfu.yijinjing.locator import Locator
from kungfu.yijinjing.log import create_logger

lf = kungfu.__binding__.longfist
yjj = kungfu.__binding__.yijinjing


kf_home = os.getenv("KF_HOME")
kfext_log_level = os.getenv("KF_LOG_LEVEL")

if not kfext_log_level:
    kfext_log_level = "error"

kfext_logger = None
if kf_home:
    kfext_log_locator = Locator(os.path.join(kf_home, "runtime"))
    kfext_log_location = yjj.location(
        lf.enums.mode.LIVE,
        lf.enums.category.SYSTEM,
        "service",
        "extensions",
        kfext_log_locator,
    )
    kfext_logger = create_logger("extensions", kfext_log_level, kfext_log_location)
else:
    kfext_logger = create_logger("extensions", kfext_log_level, None)


class ExtensionRegistry:
    def __init__(self, ext_type):
        self._registry = {}
        self.ext_type = ext_type

    def register_extension(self, name, extension):
        kfext_logger.debug("Registering %s extension %s", self.ext_type, name)
        self._registry[name] = extension

    def has_extension(self, name):
        return name in self._registry

    def get_extension(self, name):
        return self._registry[name]

    def names(self):
        return self._registry.keys()


EXTENSION_REGISTRY_MD = ExtensionRegistry("MD")
EXTENSION_REGISTRY_TD = ExtensionRegistry("TD")
EXTENSION_REGISTRY_DATA = ExtensionRegistry("DATA")
EXTENSIONS = {}
