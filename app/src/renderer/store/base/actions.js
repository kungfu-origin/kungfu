import { getAccountSource } from '__gConfig/accountConfig';
import { readJsonSync, outputJsonSync } from '__gUtils/fileUtils';
import { KF_CONFIG_PATH } from '__gConfig/pathConfig';

export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setTdAccountSource = ({ commit }, tdAccountSource) => {
    commit('SET_TD_ACCOUNT_SOURCE', tdAccountSource)
}

export const setMdAccountSource = ({ commit }, mdAccountSource) => {
    commit('SET_MD_ACCOUNT_SOURCE', mdAccountSource)
}

//初始化kfconfig
export const setKungfuConfig = ({ commit }, kfConfig) => {
    commit('SET_KF_CONFIG', kfConfig)  
    
}

//部分更新kfConfig
export const setKungfuConfigByKeys = ({ commit, state }, kfConfig) => {
    commit('SET_KF_CONFIG', kfConfig)  
    outputJsonSync(KF_CONFIG_PATH, 
        {
            ...state.kfConfig,
            ...kfConfig
        }
    )
}

export const getAccountSourceConfig = ({ dispatch }) => {
    return getAccountSource().then(accountSource => {
        dispatch('setTdAccountSource', accountSource.td)
        dispatch('setMdAccountSource', accountSource.md)
    })
}

//初始化kungfu trader
export const getKungfuConfig = ({ dispatch, state }) => {
    const kfConfig = readJsonSync(KF_CONFIG_PATH)
    dispatch('setKungfuConfig', kfConfig)
}


