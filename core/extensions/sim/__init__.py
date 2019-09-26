from .marketdata_sim import MarketDataSim
from .trader_sim import TraderSim
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD
EXTENSION_REGISTRY_MD.register_extension('sim', MarketDataSim)
EXTENSION_REGISTRY_TD.register_extension('sim', TraderSim)
