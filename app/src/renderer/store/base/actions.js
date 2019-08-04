import Vue from 'vue';
import * as BASE_API from '__io/db/base';
import { nanoGetCalendar } from '__io/nano/nanoReq';
import { getAccountSource } from '__gConfig/accountConfig';


export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

export const setSourceAccount = ({ commit }, sourceAccount) => {
    commit('SET_SOURCE_ACCOUNT', sourceAccount)
}

//主动获得交易日
export const getCalendar = ({ commit }) => {
    return nanoGetCalendar()
    .then((calendar) => {
        if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
        return calendar
    })
}

getAccountSource().then(res => {
    console.log(res,'-----')
})



