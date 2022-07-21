import './injectGlobal';
import {
  ensureKungfuKey,
  initKfConfig,
} from '@kungfu-trader/kungfu-js-api/config';

initKfConfig();
ensureKungfuKey();
