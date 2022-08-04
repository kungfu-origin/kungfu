import '@kungfu-trader/kungfu-js-api/hooks';
import pm2 from '@kungfu-trader/kungfu-js-api/utils/pm2Custom';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
globalThis.pm2 = pm2;
globalThis.globalBus = globalBus;
