import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const kf = kungfu();

kfLogger.info('Load kungfu node');

if (global.longfist === undefined) {
    global.longfist = kf.longfist;
} else {
    throw new Error('Longfist loaded again');
}

if (global.configStore === undefined) {
    global.configStore = kf.ConfigStore(KF_RUNTIME_DIR);
} else {
    throw new Error('ConfigStore loaded again');
}

if (global.commissionStore === undefined) {
    global.commissionStore = kf.CommissionStore(KF_RUNTIME_DIR);
} else {
    throw new Error('CommissionStore loaded again');
}

if (global.kfHistory === undefined) {
    global.kfHistory = kf.History(KF_RUNTIME_DIR);
} else {
    throw new Error('History loaded again');
}
