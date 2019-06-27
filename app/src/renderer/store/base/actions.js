import Vue from 'vue';
import * as BASE_API from '@/io/db/base';
import { nanoGetCalendar } from '@/io/nano/nanoReq';

export const setCalendar = ({commit}, calendar) => {
    commit('SET_CALENDAR', calendar)
}

export const setProcessStatus = ({commit}, processStatus) => {
    commit('SET_PROCESS_STATUS', processStatus)
}

//主动获得交易日
export const getCalendar = ({dispatch}) => {
    return nanoGetCalendar().then((calendar) => {
        dispatch('setCalendar', calendar)
        return calendar
    })
}




