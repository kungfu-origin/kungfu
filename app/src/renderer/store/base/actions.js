import { nanoReqCalendar } from '__io/nano/nanoReq';
import moment from 'moment'

export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setAccountSource = ({ commit }, accountSource) => {
    commit('SET_ACCOUNT_SOURCE', accountSource)
}

export const setTradingDay = ({ commit }, tradingDay) => {
    commit('SET_TRADING_DAY', tradingDay);
}

//主动获得交易日
export const reqCalendar = ({ commit }) => {
    return nanoReqCalendar()
    .then(calendar => {
        if(calendar && calendar.trading_day) {
            const tradingDay = moment(calendar.trading_day).format('YYYYMMDD');
            commit('SET_TRADING_DAY', tradingDay);
        }
        return calendar
    })
    .catch(err => console.error(err))
}
