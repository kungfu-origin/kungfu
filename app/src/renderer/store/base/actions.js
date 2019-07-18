import Vue from 'vue';
import * as BASE_API from '__io/db/base';
import { nanoGetCalendar } from '__io/nano/nanoReq';


export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

//主动获得交易日
export const getCalendar = ({ commit }) => {
    return nanoGetCalendar()
    .then((calendar) => {
        if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
        return calendar
    })
}




