const path = require('path');
const mainProcess = require('electron').app;
const renderProcess = require('electron').remote;
const {addFile} = require('__gUtils/fileUtils');

//ELEC_BASE
var ELEC_BASE_DIR_RESOLVE;
if(process.env.NODE_ENV === 'cli'){
    ELEC_BASE_DIR_RESOLVE = '/Users/zhangyizhi/Library/Application Support/kungfu';
    addFile('', ELEC_BASE_DIR_RESOLVE, 'folder')
} else {
    ELEC_BASE_DIR_RESOLVE = mainProcess ? mainProcess.getPath('userData') : renderProcess.app.getPath('userData')
    addFile('', ELEC_BASE_DIR_RESOLVE, 'folder')
}
export const ELEC_BASE_DIR = ELEC_BASE_DIR_RESOLVE;

//BASE
addFile(ELEC_BASE_DIR, 'app', 'folder')
export const BASE_DIR = path.join(ELEC_BASE_DIR, 'app')
console.log(BASE_DIR)

//GLOBAL_DIR strategys, accounts, tasks
addFile(BASE_DIR, 'global', 'folder')
export const GLOBAL_DIR = path.join(BASE_DIR, 'global');

//gateway
export const GATEWAY_DIR = path.join(BASE_DIR, 'gateway');
addFile(BASE_DIR, 'gateway', 'folder')

//accounts
addFile(BASE_DIR, 'accounts', 'folder')
export const ACCOUNTS_DIR = path.join(BASE_DIR, 'accounts');

//journal
addFile(BASE_DIR, 'journal', 'folder')
export const JOURNAL_DIR = path.join(BASE_DIR, 'journal');

//log
addFile(BASE_DIR, 'log', 'folder')
export const LOG_DIR = path.join(BASE_DIR, 'log');

//strategy
addFile(BASE_DIR, 'strategy', 'folder')
export const STRATEGY_DIR = path.join(BASE_DIR, 'strategy');

//socket
addFile(BASE_DIR, 'socket', 'folder')
export const SOCKET_DIR = path.join(BASE_DIR, 'socket');


//strategy
export const STRATEGYS_DB = path.join(GLOBAL_DIR, 'strategys.db')

//accounts
export const ACCOUNTS_DB = path.join(GLOBAL_DIR, 'accounts.db')

//tasks
export const TASKS_DB = path.join(GLOBAL_DIR, 'task.db')

//kungfu-engine
export const KUNGFU_ENGINE = process.env.NODE_ENV === 'production' 
? path.join(process.resourcesPath)
: path.join(__dirname, '..', '..', 'build')

//gateway
export const buildGatewayPath = (gatewayName) => {
    return path.join(GATEWAY_DIR, gatewayName)
}

//account folder
export const buildAccountFolderPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId)
}

//account commission 手续费
export const buildAccountCommissionDBPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId, 'commission.db')    
}

//global commission 手续费
export const buildGloablCommissionDBPath = () => {
    return path.join(GLOBAL_DIR, 'commission.db')    
}

//gateway state
export const buildGateWayStateDBPath = (gatewayName) => {
    return path.join(buildGatewayPath(gatewayName), 'state.db')
}

//assets
export const buildAccountAssetsDBPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId, 'assets.db')
}

//orders
export const buildAccountOrdersDBPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId, 'orders.db')
}

//trades
export const buildAccountTradesDBPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId, 'trades.db')
}

//snapshorts
export const buildAccountSnapshortsDBPath = (accountId) => {
    return path.join(ACCOUNTS_DIR, accountId, 'snapshots.db')
}

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

//当策略运行时，其使用的md accounts(sources)
export const buildCurrentMdAccountsByStrategyDBPath = (strategyId) => {
    return path.join(STRATEGY_DIR, strategyId, 'current_md_feed.db')
}

//当策略运行时，其使用的td accounts 
export const buildCurrentTdAccountsByStrategyDBPath = (strategyId) => {
    return path.join(STRATEGY_DIR, strategyId, 'current_account_list.db')    
}

//获取进程日志地址
export const buildProcessLogPath = (processId) => {
    return path.join(LOG_DIR, `${processId}.log`)
}
