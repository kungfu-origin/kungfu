import path from 'path';
import moment from 'moment';

import { addFileSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { KF_HOME_BASE_DIR_RESOLVE } from '@kungfu-trader/kungfu-js-api/config/homePathConfig';

addFileSync('', KF_HOME_BASE_DIR_RESOLVE, 'folder');
export const KF_HOME_BASE_DIR = KF_HOME_BASE_DIR_RESOLVE;

//BASE
export const KF_HOME = path.join(KF_HOME_BASE_DIR, 'home');
addFileSync('', KF_HOME, 'folder');

//RUNTIME
export const KF_RUNTIME_DIR = path.join(KF_HOME, 'runtime');
addFileSync('', KF_RUNTIME_DIR, 'folder');

//DATASET
export const KF_DATASET_DIR = path.join(KF_HOME, 'dataset');
addFileSync('', KF_DATASET_DIR, 'folder');

//marketData in dataSet
export const KF_DATASET_QUOTE_DIR = path.join(KF_HOME, 'dataset', 'quote');
addFileSync('', KF_DATASET_QUOTE_DIR, 'folder');

//system
export const SYSTEM_DIR = path.join(KF_RUNTIME_DIR, 'system');
addFileSync('', SYSTEM_DIR, 'folder');

//strategy
export const STRATEGY_DIR = path.join(KF_RUNTIME_DIR, 'strategy');
addFileSync('', STRATEGY_DIR, 'folder');

//td
export const TD_DIR = path.join(KF_RUNTIME_DIR, 'td');
addFileSync('', TD_DIR, 'folder');

//md
export const MD_DIR = path.join(KF_RUNTIME_DIR, 'md');
addFileSync('', MD_DIR, 'folder');

//ledger
export const LEDGER_DIR = path.join(SYSTEM_DIR, 'service', 'ledger');
addFileSync('', LEDGER_DIR, 'folder');

//log
export const LOG_DIR = path.join(KF_HOME, 'logview');
addFileSync('', LOG_DIR, 'folder');

//================= global db start ==============================

//BASE_DB_DIR strategys, accounts, tasks
export const BASE_DB_DIR = path.join(SYSTEM_DIR, 'etc', 'kungfu', 'db', 'live');

//================= account related start ==========================
//gateway
export const buildGatewayPath = (gatewayName: string) =>
    path.join(KF_RUNTIME_DIR, ...gatewayName.split('_'));

//================== others start =================================

const production = process.env.NODE_ENV === 'production';
const staticKfcDir = path.join('..', 'core', 'dist', 'kfc');

//获取进程日志地址
export const buildProcessLogPath = (processId: string) => {
    const tmk = moment().format('YYYYMMDD');
    return path.join(LOG_DIR, tmk, `${processId}.log`);
};

//================== others end ===================================

//================== config start =================================

//process.resourcesPath 是一个容易出错的问题，需要每个调用pathconfig的进程都注册了这个值，不然报错

export const KUNGFU_RESOURCES_DIR = production
    ? path.join(process.resourcesPath, 'kungfu-resources')
    : path.join(__resources);

export const KF_CONFIG_DEFAULT_PATH = path.join(
    KUNGFU_RESOURCES_DIR,
    'config',
    'kfConfig.json',
);

export const KF_TARADING_CONFIG_DEFAULT_PATH = path.join(
    KUNGFU_RESOURCES_DIR,
    'config',
    'kfTradingConfig.json',
);

export const KF_ADMIN_PASSWORD_CONFIG_DEFAULT_PATH = path.join(
    KUNGFU_RESOURCES_DIR,
    'config',
    'adminPassword.json',
);

export const KF_CONFIG_PATH = path.join(KF_HOME, 'config', 'kfConfig.json');

export const KF_TARADING_CONFIG_PATH = path.join(
    KF_HOME,
    'config',
    'kfTradingConfig.json',
);

export const KF_ADMIN_PASSWORD_CONFIG_PATH = path.join(
    KF_HOME,
    'config',
    'adminPassword.json',
);

//================== config end =================================

//================== json db start ================================

export const KF_TICKER_SET_JSON_PATH = path.join(
    KF_HOME,
    'config',
    'tickerSets.json',
);
addFileSync('', KF_TICKER_SET_JSON_PATH, 'file');

//================== json db end ==================================

export const KFC_PARENT_DIR = production
    ? process.resourcesPath
    : path.dirname(process.env.KFC_DIR || staticKfcDir);

export const KFC_DIR = process.env.KFC_DIR || path.join(KFC_PARENT_DIR, 'kfc');

export const KFC_EXECUTABLE = process.platform === 'win32' ? 'kfc.exe' : 'kfc';

export const EXTENSION_DIR = path.join(
    KFC_PARENT_DIR,
    'kfc',
    'kungfu_extensions',
);

export const TASK_EXTENSION_DIR = production
    ? path.join(process.resourcesPath, 'kungfu-extensions')
    : path.join(__dirname, '..', '..', '..');

export const APP_DIR = production
    ? path.resolve(process.resourcesPath, 'app', 'dist', 'app')
    : path.resolve(process.cwd(), 'dist', 'app');

process.env.KFC_DIR = KFC_DIR;

console.log(`process.cwd = ${process.cwd()}`);
console.log(`process.resourcesPath = ${process.resourcesPath}`);
console.log(`KFC_PARENT_DIR = ${KFC_PARENT_DIR}`);