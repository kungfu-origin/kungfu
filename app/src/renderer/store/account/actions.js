
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

export const setAccountAssetById = ({ commit },  { accountId, accountAsset }) => {
    commit('SET_ACCOUNT_ASSET',  { accountId, accountAsset })
}

export const setAccountList = ({ commit }, accountList) => {
    commit('SET_ACCOUNT_LIST', accountList)
}

//获取 accountList
export const getAccountList = ({ dispatch }) => {
    return new Promise((resolve, reject) => {
        ACCOUNT_API.getAccountList().then(res => {
            dispatch('setAccountList', res);
            resolve(res)
        })
    })
}