import { addFile } from '__gUtils/fileUtils';
const path = require('path');
const mainProcess = require('electron').app;
const renderProcess = require('electron').remote;


//ELEC_BASE
var ELEC_BASE_DIR_RESOLVE;
if(process.env.APP_TYPE === 'cli') ELEC_BASE_DIR_RESOLVE = process.env.APPDATA || (process.platform == 'darwin' ? path.join(process.env.HOME, 'Library', 'Application Support', 'kungfu') : path.join('var', 'local', 'kungfu'))
else ELEC_BASE_DIR_RESOLVE = mainProcess ? mainProcess.getPath('userData') : renderProcess.app.getPath('userData')
addFile('', ELEC_BASE_DIR_RESOLVE, 'folder')

export const ELEC_BASE_DIR = ELEC_BASE_DIR_RESOLVE;

//BASE
export const KF_HOME = path.join(ELEC_BASE_DIR, 'app')
addFile('', KF_HOME, 'folder')

//system
export const SYSTEM_DIR = path.join(KF_HOME, 'system');
addFile('', SYSTEM_DIR, 'folder')

//runtime
export const RUNTIME_DIR = path.join(KF_HOME, 'runtime');
addFile('', RUNTIME_DIR, 'folder')

//strategy
export const STRATEGY_DIR = path.join(KF_HOME, 'strategy');
addFile('', STRATEGY_DIR, 'folder')

//td
export const TD_DIR = path.join(KF_HOME, 'td');
addFile('', TD_DIR, 'folder')

//md
export const MD_DIR = path.join(KF_HOME, 'md');
addFile('', MD_DIR, 'folder')

//watcher 
export const WATCHER_DIR = path.join(SYSTEM_DIR, 'watcher', 'watcher')
addFile('', WATCHER_DIR, 'folder')

//log
export const LOG_DIR = path.join(KF_HOME, 'log');
addFile('', LOG_DIR, 'folder')


//================= global db start ==============================

//BASE_DB_DIR strategys, accounts, tasks
export const BASE_DB_DIR = path.join(SYSTEM_DIR, 'etc', 'kungfu', 'db', 'live');

//strategy
export const STRATEGYS_DB = path.join(BASE_DB_DIR, 'strategys.db')

//accounts(td)
export const ACCOUNTS_DB = path.join(BASE_DB_DIR, 'accounts.db')

//tasks
export const TASKS_DB = path.join(BASE_DB_DIR, 'task.db')

//================= global db end =================================

//================= account related start ==========================

//gateway
export const buildGatewayPath = (gatewayName: string) => path.join(KF_HOME, ...gatewayName.split('_'))

//gateway data
export const buildGatewayLiveDBPath = (gatewayName: string) => path.join(buildGatewayPath(gatewayName), 'db', 'live')

//account commission 手续费
export const buildAccountCommissionDBPath = (accountId: string) => path.join(buildGatewayLiveDBPath(`td_${accountId}`), 'commission.db')    

//================= account related end ==========================

//================= live trading start ===========================

//trading data
export const LIVE_TRADING_DB_DIR = path.join(WATCHER_DIR, 'db', 'live');

export const LIVE_TRADING_DATA_DB = path.join(LIVE_TRADING_DB_DIR, 'watcher.db')

//================= live trading end =============================

//================== others start =================================

//global commission 手续费
export const GLOBAL_COMMISSION_DB = path.join(BASE_DB_DIR, 'commission.db');

//获取进程日志地址
export const buildProcessLogPath = (processId: string) => path.join(LOG_DIR, `${processId}.log`)

//获取watcher nano pub 地址
export const NMSG_PUB_FILE = path.join(SYSTEM_DIR, 'watcher', 'watcher', 'nn', 'live', 'pub.nn')

//获取watcher nano rep 地址
export const NMSG_REP_FILE = path.join(SYSTEM_DIR, 'watcher', 'watcher', 'nn', 'live', 'rep.nn')

//================== others end ===================================

//kungfu-engine
export const KUNGFU_ENGINE: string = process.env.NODE_ENV === 'production' 
    ? process.env.APP_TYPE === 'cli' 
        ? path.join('.') 
        //@ts-ignore
        : process.resourcesPath
    : process.env.APP_TYPE === 'cli' 
        ? path.join(__dirname, '..', '..', '..', '..', 'core', 'build')
        : path.join(__dirname, '..', '..', '..', 'core', 'build')