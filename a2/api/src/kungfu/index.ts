import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import { kfLogger } from '../utils/busiUtils';

export const kf = kungfu();
kfLogger.info('Load kungfu node');

export const configStore = kf.ConfigStore(KF_RUNTIME_DIR);
export const kungfuShutdown = kf.shutdown;

export const dealKfTime = (nano: bigint, date = false): string => {
    if (date) {
        return kf.formatTime(nano, '%m/%d %H:%M:%S');
    }
    return kf.formatTime(nano, '%H:%M:%S');
};
