import Vue from 'vue';
export const SET_ONE_MD_TD_STATE = (state, { id, stateData }) => {
    Vue.set(state.mdTdState, id, stateData)
}

export const DELETE_ONE_MD_TD_STATE = (state, id) => {
    delete state.mdTdState[id]
    Vue.delete(state.mdTdState, id)    
}

export const SET_ACCOUNTS_ASSET = (state, accountsAsset) => {
    state.accountsAsset = accountsAsset
}

export const SET_ACCOUNT_ASSET = (state, { accountId, accountsAsset }) => {
    Vue.set(state.accountsAsset, accountId, accountsAsset)
}

export const SET_TD_LIST = (state, tdList) => {
    state.tdList = tdList
}

export const SET_MD_LIST = (state, mdList) => {
    state.mdList = mdList
}

export const SET_CURRENT_ACCOUNT = (state, account) => {
    state.currentAccount = account;
}
