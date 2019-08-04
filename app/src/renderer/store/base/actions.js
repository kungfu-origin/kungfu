<<<<<<< HEAD
import { nanoReqCalendar } from '__io/nano/nanoReq';
=======
import Vue from 'vue';
import * as BASE_API from '__io/db/base';
import { nanoGetCalendar } from '__io/nano/nanoReq';
import { getAccountSource } from '__gConfig/accountConfig';
>>>>>>> [add] fix extension


export const setProcessStatus = ({ commit }, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

<<<<<<< HEAD
export const setAccountSource = ({ commit }, accountSource) => {
    commit('SET_ACCOUNT_SOURCE', accountSource)
=======
export const setSourceAccount = ({ commit }, sourceAccount) => {
    commit('SET_SOURCE_ACCOUNT', sourceAccount)
>>>>>>> [add] fix extension
}

//主动获得交易日
export const reqCalendar = ({ commit }) => {
    return nanoReqCalendar()
    .then((calendar) => {
        if(calendar && calendar.trading_day) commit('SET_CALENDAR', calendar);
        return calendar
    })
}

getAccountSource().then(res => {
    console.log(res,'-----')
})


