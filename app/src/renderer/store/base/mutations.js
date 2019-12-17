import Vue from 'vue';

export const SET_TASKS = (state, tasks) => {
    state.tasks = tasks
}

export const SET_TRADING_DAY = (state, tradingDay) => {
    state.tradingDay = tradingDay
}

export const SET_PROCESS_STATUS = (state, processStatus) => {
    state.processStatus = processStatus
}

export const SET_TD_ACCOUNT_SOURCE = (state, tdAccountSource) => {
    state.tdAccountSource = tdAccountSource
}

export const SET_MD_ACCOUNT_SOURCE = (state, mdAccountSource) => {
    state.mdAccountSource = mdAccountSource
}

export const SET_KF_CONFIG = (state, kfConfig) => {
    Object.keys(kfConfig || {}).forEach(key => {
        Vue.set(state.kfConfig, key, kfConfig[key])
    })
}