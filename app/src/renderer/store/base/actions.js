import { nanoReqCalendar } from '__io/nano/nanoReq';
import { buildTradingDayPipe } from '__io/nano/nanoSub';

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
        if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
        return calendar
    })
    .catch(err => console.error(err))
}

//sub 交易日
buildTradingDayPipe().subscribe(d => {
    const calendar = d.data;
    console.log('sub calendar', calendar)
    if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
})