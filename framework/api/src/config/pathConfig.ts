import path from 'path';
import dayjs from 'dayjs';

import { addFileSync } from '../utils/fileUtils';
import { KF_HOME_BASE_DIR_RESOLVE } from '../config/homePathConfig';

declare const global: NodeJS.Global;

addFileSync('', KF_HOME_BASE_DIR_RESOLVE, 'folder');
export const KF_HOME_BASE_DIR = KF_HOME_BASE_DIR_RESOLVE;

//BASE
export const KF_HOME = path.join(KF_HOME_BASE_DIR, 'home');
addFileSync('', KF_HOME, 'folder');

//RUNTIME
export const KF_RUNTIME_DIR = path.join(KF_HOME, 'runtime');
addFileSync('', KF_RUNTIME_DIR, 'folder');

//system
export const SYSTEM_DIR = path.join(KF_RUNTIME_DIR, 'system');
addFileSync('', SYSTEM_DIR, 'folder');

//log
export const LOG_DIR = path.join(KF_HOME, 'logview');
addFileSync('', LOG_DIR, 'folder');

//================= global db start ==============================

//BASE_DB_DIR strategys, accounts, tasks
export const BASE_DB_DIR = path.join(SYSTEM_DIR, 'etc', 'kungfu', 'db', 'live');

//================== others start =================================

const production = process.env.NODE_ENV === 'production';
const staticKfcDir = path.join('..', 'core', 'dist', 'kfc');

//获取进程日志地址
export const buildProcessLogPath = (processId: string) => {
    const tmk = dayjs().format('YYYYMMDD');
    return path.join(LOG_DIR, tmk, `${processId}.log`);
};

//================== others end ===================================

//================== config start =================================

//global.__kfResourcesPath 是一个容易出错的问题，需要每个调用pathconfig的进程都注册了这个值，不然报错
export const KUNGFU_RESOURCES_DIR = production
    ? path.join(global.__kfResourcesPath, 'kungfu-resources')
    : path.join(global.__resources);

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

export const KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH = path.join(
    KF_HOME,
    'config',
    'subscribedInstruments.json',
);
addFileSync('', KF_SUBSCRIBED_INSTRUMENTS_JSON_PATH, 'file');

//================== json db end ==================================

export const KFC_PARENT_DIR = production
    ? global.__kfResourcesPath
    : path.dirname(process.env.KFC_DIR || staticKfcDir);

export const KFC_DIR = process.env.KFC_DIR || path.join(KFC_PARENT_DIR, 'kfc');

export const KFC_EXECUTABLE = process.platform === 'win32' ? 'kfc.exe' : 'kfc';

export const EXTENSION_DIR = production
    ? path.join(KFC_DIR, 'kungfu_extensions')
    : path.resolve(KFC_PARENT_DIR, '..', '..', '..', 'extensions');

export const TASK_EXTENSION_DIR = production
    ? path.join(global.__kfResourcesPath, 'kungfu-extensions')
    : path.join(__dirname, '..', '..', '..');

export const APP_DIR = production
    ? path.resolve(global.__kfResourcesPath, 'app', 'dist', 'app')
    : path.resolve(process.cwd(), 'dist', 'app');

process.env.KFC_DIR = KFC_DIR;

console.log(`process.cwd = ${process.cwd()}`);
console.log(`__kfResourcesPath = ${global.__kfResourcesPath}`);
console.log(`__resources = ${global.__resources}`);
console.log(`KFC_DIR = ${KFC_DIR}`);
console.log(`EXTENSION_DIR = ${EXTENSION_DIR}`);

//DATASET
export const KF_DATASET_DIR = path.join(KF_HOME, 'dataset');
addFileSync('', KF_DATASET_DIR, 'folder');

//marketData in dataSet
export const KF_DATASET_QUOTE_DIR = path.join(KF_HOME, 'dataset', 'quote');
addFileSync('', KF_DATASET_QUOTE_DIR, 'folder');
