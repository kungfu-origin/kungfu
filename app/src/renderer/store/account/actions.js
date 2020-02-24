
import Vue from 'vue';
import * as ACCOUNT_API from '__io/db/account';
import { debounce } from '__gUtils/busiUtils';

//保存选中的账户信息
export const setCurrentAccount = ({ commit }, account) => {
    commit('SET_CURRENT_ACCOUNT', account)
}

export const setOneMdTdState = ({ commit },  { id, stateData }) => {
    commit('SET_ONE_MD_TD_STATE',  { id, stateData })
}

//关闭状态删除某个状态
export const deleteOneMdTdState = ({ commit },  id) => {
    commit('DELETE_ONE_MD_TD_STATE',  id)
}

export const setAccountsAsset = ({ commit }, accountsAsset) => {
    commit('SET_ACCOUNTS_ASSET', accountsAsset)
}

export const setTdList = ({ commit }, tdList) => {
    commit('SET_TD_LIST', tdList)
}

export const setMdList = ({ commit }, mdList) => {
    commit('SET_MD_LIST', mdList)
}

//获取 tdList
export const getTdList = ({ dispatch }) => {
    return new Promise((resolve, reject) => {
        ACCOUNT_API.getTdList()
        .then(res => {
            dispatch('setTdList', res);
            resolve(res)
        })
        .catch(err => {
            console.error(err)
        })
    })
}

//获取 mdList
export const getMdList = ({ dispatch }) => {
    return new Promise((resolve, reject) => {
        ACCOUNT_API.getMdList()
        .then(res => {
            dispatch('setMdList', res);
            resolve(res)
        })
        .catch(err => {
            console.error(err)
        })
    })
}