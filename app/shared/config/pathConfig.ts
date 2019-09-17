import { addFile } from '__gUtils/fileUtils';
import { platform } from './platformConfig';
const path = require('path');
const mainProcess = require('electron').app;
const renderProcess = (require('electron').remote || {}).app;

if(mainProcess) {
    const appName = 'kungfu'
    mainProcess.setName('kungfu')
    const appData = mainProcess.getPath('appData');
    mainProcess.setPath('userData', path.join(appData, appName));    
}

//ELEC_BASE
var ELEC_BASE_DIR_RESOLVE;
if (process.env.APP_TYPE === 'test') ELEC_BASE_DIR_RESOLVE = process.env.ELEC_BASE_DIR || ''; 
else ELEC_BASE_DIR_RESOLVE = mainProcess ? mainProcess.getPath('userData') : renderProcess.getPath('userData');

addFile('', ELEC_BASE_DIR_RESOLVE, 'folder');

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

//ledger 
export const LEDGER_DIR = path.join(SYSTEM_DIR, 'service', 'ledger')
addFile('', LEDGER_DIR, 'folder')

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
export const LIVE_TRADING_DB_DIR = path.join(LEDGER_DIR, 'db', 'live');

export const LIVE_TRADING_DATA_DB = path.join(LIVE_TRADING_DB_DIR, 'ledger.db')

//================= live trading end =============================

//================== others start =================================

//global commission 手续费
export const GLOBAL_COMMISSION_DB = path.join(BASE_DB_DIR, 'commission.db');

//获取进程日志地址
export const buildProcessLogPath = (processId: string) => path.join(LOG_DIR, `${processId}.log`)

//获取 ledger nano pub 地址
export const NMSG_PUB_FILE = path.join(SYSTEM_DIR, 'service', 'ledger', 'nn', 'live', 'pub.nn')

//获取 ledger nano rep 地址
export const NMSG_REP_FILE = path.join(SYSTEM_DIR, 'service', 'ledger', 'nn', 'live', 'rep.nn')

//================== others end ===================================

//kungfu-engine
var KUNGFU_ENGINE_RESOLVE: string = process.env.NODE_ENV === 'production' 
    //@ts-ignore
    ? process.resourcesPath
    : path.join(__dirname, '..', '..', '..', 'core', 'build')
if(process.env.APP_TYPE === 'test') KUNGFU_ENGINE_RESOLVE = process.env.KUNGFU_ENGINE_PATH || ''
export const KUNGFU_ENGINE_PATH = KUNGFU_ENGINE_RESOLVE;

const KUNGFU_RESOURCES_DIR = process.env.NODE_ENV === 'production' 
//@ts-ignore
? path.join(process.resourcesPath, 'kungfu-resources') 
: path.join(__resources)

export const KF_CONFIG_PATH = path.join(KUNGFU_RESOURCES_DIR, 'config', 'kfConfig.json')

export const DEFUALT_DB_PATH = path.join(KUNGFU_RESOURCES_DIR, 'default')

export const EXTENSION_DIR = path.join(KUNGFU_ENGINE_PATH, 'kfc', 'extensions');
