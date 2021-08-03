from extensions.sim.src.python.broker import MarketDataSim
from extensions.sim.src.python.broker import TraderSim
from kungfu_extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD

EXTENSION_REGISTRY_MD.register_extension("sim", MarketDataSim)
EXTENSION_REGISTRY_TD.register_extension("sim", TraderSim)
