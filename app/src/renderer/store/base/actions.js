import { getAccountSource } from '__gConfig/accountConfig';


export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setAccountSource = ({ commit }, accountSource) => {
    commit('SET_ACCOUNT_SOURCE', accountSource)
}

export const setTradingDay = ({ commit }, tradingDay) => {
    commit('SET_TRADING_DAY', tradingDay);
}

export const getAccountSourceConfig = ({ dispatch }) => {
    return getAccountSource().then(res => {
        dispatch('setAccountSource', res)
    })
}