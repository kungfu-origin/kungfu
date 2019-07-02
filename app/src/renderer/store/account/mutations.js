export const SET_ONE_MD_TD_STATE = (state, { id, stateData }) => {
    state.mdTdState[id] = stateData
}

export const DELETE_ONE_MD_TD_STATE = (state, id) => {
    delete state.mdTdState[id]
}

export const SET_ACCOUNTS_ASSET = (state, accountsAsset) => {
    state.accountsAsset = accountsAsset
}

export const SET_ACCOUNT_ASSET = (state, {accountId, accountAsset}) => {
    state.accountsAsset[accountId] = accountAsset
}

export const SET_ACCOUNT_LIST = (state, accountList) => {
    state.accountList = accountList
}


export const SET_CURRENT_ACCOUNT = (state, account) => {
    state.currentAccount = account;
}
