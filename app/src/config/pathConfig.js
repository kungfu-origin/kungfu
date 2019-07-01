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
addFile(ELEC_BASE_DIR, 'app', 'folder')
export const BASE_DIR = path.join(ELEC_BASE_DIR, 'app')

//system
addFile(BASE_DIR, 'system', 'folder')
export const SYSTEM_DIR = path.join(BASE_DIR, 'system');

//GLOBAL_DIR strategys, accounts, tasks
export const GLOBAL_DIR = path.join(SYSTEM_DIR, 'etc', 'kungfu', 'db', 'live');

//runtime
addFile(BASE_DIR, 'runtime', 'folder')
export const RUNTIME_DIR = path.join(BASE_DIR, 'runtime');


//strategy
addFile(BASE_DIR, 'strategy', 'folder')
export const STRATEGY_DIR = path.join(BASE_DIR, 'strategy');

//accounts(td)
addFile(BASE_DIR, 'td', 'folder')
export const ACCOUNTS_DIR = path.join(BASE_DIR, 'td');

//md
addFile(BASE_DIR, 'md', 'folder')
export const MD_DIR = path.join(BASE_DIR, 'md');

//log
addFile(BASE_DIR, 'log', 'folder')
export const LOG_DIR = path.join(BASE_DIR, 'log');


// //socket
// addFile(BASE_DIR, 'socket', 'folder')
// export const SOCKET_DIR = path.join(BASE_DIR, 'socket');
export const SOCKET_DIR = '';

//gateway
export const GATEWAY_DIR = ''





//================= global db start ==============================

//strategy
export const STRATEGYS_DB = path.join(GLOBAL_DIR, 'strategys.db')

//accounts(td)
export const ACCOUNTS_DB = path.join(GLOBAL_DIR, 'accounts.db')

//tasks
export const TASKS_DB = path.join(GLOBAL_DIR, 'task.db')

//================= global db end =================================

//================= account related start ==========================

//gateway
export const buildGatewayPath = (gatewayName) => {
    return path.join(BASE_DIR, ...gatewayName.split('_'))
}

//gateway state
export const buildGateWayStateDBPath = (gatewayName) => {
    return path.join(buildGatewayPath(gatewayName), 'db', 'live', 'state.db')
}

//account folder
export const buildAccountFolderPath = (accountId) => {
    const {source, id} = accountId.parseSourceAccountId();
    return path.join(ACCOUNTS_DIR, source, id, 'db', 'live')
}

//account commission 手续费
export const buildAccountCommissionDBPath = (accountId) => {
    return path.join(buildAccountFolderPath(accountId), 'commission.db')    
}

//assets
export const buildAccountAssetsDBPath = (accountId) => {
    return path.join(buildAccountFolderPath(accountId), 'assets.db')
}

//orders
export const buildAccountOrdersDBPath = (accountId) => {
    return path.join(buildAccountFolderPath(accountId), 'orders.db')
}

//trades
export const buildAccountTradesDBPath = (accountId) => {
    return path.join(buildAccountFolderPath(accountId), 'trades.db')
}

//snapshorts
export const buildAccountSnapshortsDBPath = (accountId) => {
    return path.join(buildAccountFolderPath(accountId), 'snapshots.db')
}

//================= account related end ==========================

//================= strategy related start =======================

//strategyAccounts 某策略下的账户
export const buildStrategyAccountsDBPath = (strategyId) => {
    return path.join(STRATEGY_DIR, strategyId, 'account_list.db')
}

//strategyPos 某策略下的持仓
export const buildStrategyPosDBPath = (strategyId) => {
    return path.join(STRATEGY_DIR, strategyId, 'assets.db')
}

//snapshorts
export const buildStrategySnapshortsDBPath = (accountId) => {
    return path.join(STRATEGY_DIR, accountId, 'snapshots.db')
}

//================== strategy related end =========================

//================== others start =================================

//global commission 手续费
export const buildGloablCommissionDBPath = () => {
    return path.join(GLOBAL_DIR, 'commission.db')    
}

//获取进程日志地址
export const buildProcessLogPath = (processId) => {
    return path.join(LOG_DIR, `${processId}.log`)
}

//获取watcher nano pub 地址
export const buildNmsgPubFilePath = () => {
    return path.join(SYSTEM_DIR, 'watcher', 'watcher', 'nn', 'live', 'pub.nn')
}

//获取watcher nano rep 地址
export const buildNmsgRepFilePath = () => {
    return path.join(SYSTEM_DIR, 'watcher', 'watcher', 'nn', 'live', 'rep.nn')
}

//================== others end ===================================

//kungfu-engine
export const KUNGFU_ENGINE = process.env.NODE_ENV === 'production' 
? process.env.APP_TYPE === 'cli' ? path.join('.') : process.resourcesPath
: path.join(__dirname, '..', '..', '..', 'core', 'build')
