import { nanoReqCalendar } from '__io/nano/nanoReq';


export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setAccountSource = ({ commit }, accountSource) => {
    commit('SET_ACCOUNT_SOURCE', accountSource)
}

//主动获得交易日
export const reqCalendar = ({ commit }) => {
    return nanoReqCalendar()
    .then(calendar => {
        if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
        return calendar
    })
}


