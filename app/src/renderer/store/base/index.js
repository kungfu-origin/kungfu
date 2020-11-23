import Vue from 'vue';
import { getAccountSource } from '__gConfig/accountConfig';
import { readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { KF_CONFIG_PATH } from '__gConfig/pathConfig';


export default {
    
    state: {
        tasks: null, // 系统内正在运行的tasks
        calendar: { //交易日等相关日期信息
           
        },
        processStatus: null,
        tdAccountSource: {}, //账户柜台信息
        mdAccountSource: {}, //账户柜台信息
        kfConfig: {} // kf 配置
    },

    actions: {

        setProcessStatus ({ commit }, processStatus) {
            commit('SET_PROCESS_STATUS', processStatus)
        },

        setTdAccountSource ({ commit }, tdAccountSource) {
            commit('SET_TD_ACCOUNT_SOURCE', tdAccountSource)
        },

        setMdAccountSource ({ commit }, mdAccountSource) {
            commit('SET_MD_ACCOUNT_SOURCE', mdAccountSource)
        },

        //初始化kfconfig
        setKungfuConfig ({ commit }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            
        },

        //部分更新kfConfig
        setKungfuConfigByKeys ({ commit, state }, kfConfig) {
            commit('SET_KF_CONFIG', kfConfig)  
            outputJsonSync(KF_CONFIG_PATH, 
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
                return accountSource
            })
        },

        //初始化kungfu trader
        getKungfuConfig ({ dispatch }) {
            const kfConfig = readJsonSync(KF_CONFIG_PATH)
            dispatch('setKungfuConfig', kfConfig)
        },
    },

    mutations: {

        SET_TASKS (state, tasks) {
            state.tasks = tasks
        },
        
        SET_PROCESS_STATUS (state, processStatus) {
            state.processStatus = processStatus
        },
        
        SET_TD_ACCOUNT_SOURCE (state, tdAccountSource) {
            state.tdAccountSource = tdAccountSource
        },
        
        SET_MD_ACCOUNT_SOURCE (state, mdAccountSource) {
            state.mdAccountSource = mdAccountSource
        },
        
        SET_KF_CONFIG (state, kfConfig) {
            Object.keys(kfConfig || {}).forEach(key => {
                Vue.set(state.kfConfig, key, kfConfig[key])
            })
        }
    },

    getters: {

        ifMasterLedgerRunning (state) {
            const processStatus = state.processStatus || {};
            const masterIsRunning = processStatus.master === 'online';
            const ledgerIsRunning = processStatus.ledger === 'online';
        
            return masterIsRunning && ledgerIsRunning
        }
    }
}
