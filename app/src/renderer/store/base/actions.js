import { getAccountSource } from '__gConfig/accountConfig';
import { readJsonSync, outputJson } from '__gUtils/fileUtils';
import { KF_CONFIG_PATH } from '__gConfig/pathConfig';

export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setAccountSource = ({ commit }, accountSource) => {
    commit('SET_ACCOUNT_SOURCE', accountSource)
}

export const setTradingDay = ({ commit }, tradingDay) => {
    commit('SET_TRADING_DAY', tradingDay);
}

//初始化kfconfig
export const setKungfuConfig = ({ commit }, kfConfig) => {
    commit('SET_KF_CONFIG', kfConfig)  
    
}

//部分更新kfConfig
export const setKungfuConfigByKeys = ({ commit, state }, kfConfig) => {
    commit('SET_KF_CONFIG', kfConfig)  
    outputJson(KF_CONFIG_PATH, 
        {
            ...state.kfConfig,
            ...kfConfig
        }
    )
}

export const getAccountSourceConfig = ({ dispatch }) => {
    return getAccountSource().then(res => {
        dispatch('setAccountSource', res)
    })
}

//初始化kungfu trader
export const getKungfuConfig = ({ dispatch, state }) => {
    const kfConfig = readJsonSync(KF_CONFIG_PATH)
    dispatch('setKungfuConfig', kfConfig)
}


