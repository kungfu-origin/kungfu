import {
  ensureKungfuKey,
  initKfConfig,
} from '@kungfu-trader/kungfu-js-api/config';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';

process.env.KF_HOME = KF_RUNTIME_DIR;

initKfConfig();
ensureKungfuKey();
