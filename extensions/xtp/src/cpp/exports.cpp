#include "marketdata_xtp.h"
#include "trader_xtp.h"

#include <kungfu/wingchun/extension.h>

KUNGFU_EXTENSION(xtp) {
  KUNGFU_DEFINE_MD(kungfu::wingchun::xtp::MarketDataXTP);
  KUNGFU_DEFINE_TD(kungfu::wingchun::xtp::TraderXTP);
}
