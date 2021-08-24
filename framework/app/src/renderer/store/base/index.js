import Vue from 'vue';
import fse from "fs-extra";
import { getAccountSource } from '@kungfu-trader/kungfu-uicc/config/accountConfig';
import { KF_CONFIG_PATH, TASK_EXTENSION_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { getExtensionConfigs } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';


export default {
    
    state: {
        tasks: null, // 系统内正在运行的tasks
        calendar: { //交易日等相关日期信息
           
        },
        processStatus: {},
        processStatusWithDetail: {},
        tdAccountSource: {}, //账户柜台信息
        mdAccountSource: {}, //账户柜台信息
        riskAccountSource: {}, //账户风控信息
        kfConfig: {}, // kf 
        
        currentTask: {},
        taskExtConfigList: [],

        daemonIsLive: false,

        loginInfo: Object.freeze({}),
    },

    actions: {

        setProcessStatus ({ commit }, processStatus) {
            commit('SET_PROCESS_STATUS', processStatus)
        },

        setProcessStatusWithDetail ({ commit }, processStatusWithDetail) {
            commit('SET_PROCESS_STATUS_WITH_DETAIL', processStatusWithDetail)
        },

        setTdAccountSource ({ commit }, tdAccountSource) {
            commit('SET_TD_ACCOUNT_SOURCE', tdAccountSource)
        },

        setMdAccountSource ({ commit }, mdAccountSource) {
            commit('SET_MD_ACCOUNT_SOURCE', mdAccountSource)
        },

        setRiskAccountSource ({ commit }, riskAccountSource) {
            commit("SET_RISK_ACCOUNT_SOURCE", riskAccountSource)
        },

        //初始化kfconfig
        setKungfuConfig ({ commit }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            
        },

        //部分更新kfConfig
        setKungfuConfigByKeys ({ commit, state }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            fse.outputJsonSync(KF_CONFIG_PATH, 
                {
                    ...state.kfConfig,
                    ...kfConfig
                }
            )
        },

        getAccountSourceConfig ({ dispatch }) {
            return getAccountSource()
            .then(accountSource => {
                dispatch('setTdAccountSource', accountSource.td)
                dispatch('setMdAccountSource', accountSource.md)
                dispatch('setRiskAccountSource', accountSource.risk)
                return accountSource
            })
        },

        //初始化kungfu trader
        getKungfuConfig ({ dispatch }) {
            const kfConfig = fse.readJsonSync(KF_CONFIG_PATH)
            dispatch('setKungfuConfig', kfConfig)
        },

        setCurrentTask ({ commit }, currentTask) {
            commit('SET_CURRENT_TASK', currentTask)
        },

        getExtensionConfigs ({ commit }) {
            return getExtensionConfigs(TASK_EXTENSION_DIR)
                .then(exts => {
                    commit('SET_TASK_EXTENSION_CONFIGS', Object.freeze(exts.filter(({ type }) => type === 'task')))
                })
        },

        setDaemonIsLive ({ commit }, daemonIsLive) {
            commit("SET_DAEMON_IS_LIVE", daemonIsLive);
        },

        setLoginInfo ({ commit }, loginInfo) {
            localStorage.setItem('login_info', JSON.stringify(loginInfo));
            localStorage.setItem('login_token', loginInfo.token || "");
            commit('SET_LOGIN_INFO', loginInfo)
        }
    },

    mutations: {
        SET_LOGIN_INFO (state, loginInfo) {
            state.loginInfo = loginInfo;
        },

        SET_DAEMON_IS_LIVE (state, daemonIsLive) {
            state.daemonIsLive = daemonIsLive;
        },

        SET_TASK_EXTENSION_CONFIGS (state, taskExtConfigList) {
            state.taskExtConfigList = taskExtConfigList
        },

        SET_CURRENT_TASK (state, task) {
            state.currentTask = Object.freeze(task);
        },
        
        SET_PROCESS_STATUS (state, processStatus) {
            state.processStatus = processStatus
        },

        SET_PROCESS_STATUS_WITH_DETAIL (state, processStatusWithDetail) {
            state.processStatusWithDetail = processStatusWithDetail
        },
        
        SET_TD_ACCOUNT_SOURCE (state, tdAccountSource) {
            state.tdAccountSource = tdAccountSource
        },
        
        SET_MD_ACCOUNT_SOURCE (state, mdAccountSource) {
            state.mdAccountSource = mdAccountSource
        },

        SET_RISK_ACCOUNT_SOURCE (state, riskAccountSource) {
            state.riskAccountSource = riskAccountSource
        },
        
        SET_KF_CONFIG (state, kfConfig) {
            Object.keys(kfConfig || {}).forEach(key => {
                Vue.set(state.kfConfig, key, kfConfig[key])
            })
        }
    },

    getters: {

        proMode (state) {
            return !!(state.taskExtConfigList || []).length
        },

        taskExtMinimistConfig (state) {
            const stringTypes = ['instrumentIdInTickerSet', 'tickerSet', 'instrumentId', 'account']
            let minimistConfig = {
                string: [],
                boolean: [],
            };
            state.taskExtConfigList.forEach(config => {
                const c = config.config;
                c.forEach(item => {
                    const { key, type } = item;
                
                    if (stringTypes.includes(type)) {
                        if (!minimistConfig.string.includes(key)) {
                            minimistConfig.string.push(key)                        
                        }
                    }

                    if (type === 'bool') {
                        if (!minimistConfig.boolean.includes(key)) {
                            minimistConfig.boolean.push(key)
                        }
                    }
                })
            })

            return minimistConfig
        },

        ifMasterLedgerRunning (state) {
            const processStatus = state.processStatus || {};
            const masterIsRunning = processStatus.master === 'online';
            const ledgerIsRunning = processStatus.ledger === 'online';
        
            return masterIsRunning && ledgerIsRunning
        }
    }
}
