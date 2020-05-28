/*
 * @Author: your name
 * @Date: 2020-05-28 15:00:46
 * @LastEditTime: 2020-05-28 15:46:55
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/shared/config/pathConfig.ts
 */ 
const path = require('path');
const { addFileSync } = require('__gUtils/fileUtils');

const KF_HOME_BASE_DIR_RESOLVE = (() => {
    if ( process.env.APP_TYPE === 'cli' ) {
        return require('__gConfig/cliKfHomePathConfig').KF_HOME_BASE_DIR_RESOLVE
     } else {
        return require('__gConfig/appKfHomePathConfig').KF_HOME_BASE_DIR_RESOLVE
     }
})()

addFileSync('', KF_HOME_BASE_DIR_RESOLVE, 'folder');
export const KF_HOME_BASE_DIR = KF_HOME_BASE_DIR_RESOLVE;

//BASE
export const KF_HOME = path.join(KF_HOME_BASE_DIR, 'home')
addFileSync('', KF_HOME, 'folder')

//RUNTIME
export const KF_RUNTIME_DIR = path.join(KF_HOME, 'runtime')
addFileSync('', KF_RUNTIME_DIR, 'folder')

//system
export const SYSTEM_DIR = path.join(KF_RUNTIME_DIR, 'system');
addFileSync('', SYSTEM_DIR, 'folder')

//strategy
export const STRATEGY_DIR = path.join(KF_RUNTIME_DIR, 'strategy');
addFileSync('', STRATEGY_DIR, 'folder')

//td
export const TD_DIR = path.join(KF_RUNTIME_DIR, 'td');
addFileSync('', TD_DIR, 'folder')

//md
export const MD_DIR = path.join(KF_RUNTIME_DIR, 'md');
addFileSync('', MD_DIR, 'folder')

//ledger
export const LEDGER_DIR = path.join(SYSTEM_DIR, 'service', 'ledger')
addFileSync('', LEDGER_DIR, 'folder')

//log
export const LOG_DIR = path.join(KF_HOME, 'log');
addFileSync('', LOG_DIR, 'folder')


//================= global db start ==============================

//BASE_DB_DIR strategys, accounts, tasks
export const BASE_DB_DIR = path.join(SYSTEM_DIR, 'etc', 'kungfu', 'db', 'live');


//================= account related start ==========================
//gateway
export const buildGatewayPath = (gatewayName: string) => path.join(KF_HOME, ...gatewayName.split('_'))


//================== others start =================================

//获取进程日志地址
export const buildProcessLogPath = (processId: string) => path.join(LOG_DIR, `${processId}.log`)



//================== others end ===================================

export const getKfEnginePath = () => {
    if(process.env.NODE_ENV === 'production') {
        if(process.env.APP_TYPE === 'test'){
            return process.env.KUNGFU_ENGINE_PATH || ''
        }
    }
}

export const KUNGFU_ENGINE_PATH = process.env.NODE_ENV === 'production' 
    ? process.resourcesPath
    : path.join(__dirname, '..', '..', '..', 'core', 'build')

export const KUNGFU_RESOURCES_DIR = process.env.NODE_ENV === 'production'
    ? path.join(process.resourcesPath, 'kungfu-resources')
    : path.join(__resources)


export const KF_CONFIG_DEFAULT_PATH = path.join(KUNGFU_RESOURCES_DIR, 'config', 'kfConfig.json')

export const KF_TARADING_CONFIG_DEFAULT_PATH = path.join(KUNGFU_RESOURCES_DIR, 'config', 'kfTradingConfig.json')

export const KF_CONFIG_PATH = path.join(KF_HOME, 'config', 'kfConfig.json')

export const KF_TARADING_CONFIG_PATH = path.join(KF_HOME, 'config', 'kfTradingConfig.json')

export const DEFUALT_DB_DIR = path.join(KUNGFU_RESOURCES_DIR, 'default')

export const EXTENSION_DIR = path.join(KUNGFU_ENGINE_PATH, 'kfc', 'kungfu_extensions');
