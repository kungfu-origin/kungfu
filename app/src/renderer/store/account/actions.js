
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
