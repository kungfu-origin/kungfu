
import Vue from 'vue';
import { getTdList, getMdList } from '__io/kungfu/account';

export default {

    state: {
        mdTdState: {},
        accountsAsset: {},
        tdList: [],
        mdList: [],
        currentAccount: {},
        currentAccountTabName: 'tdList',
        currentTicker: {}
    },

    actions: {

        //保存选中的账户信息
        setCurrentAccount ({ commit }, account) {
            commit('SET_CURRENT_ACCOUNT', account)
        },

        setCurrentTicker ({ commit }, ticker) {
            commit('SET_CURRENT_TICKER', ticker)
        },
 
        setOneMdTdState ({ commit },  { id, stateData }) {
            commit('SET_ONE_MD_TD_STATE',  { id, stateData })
        },

        //关闭状态删除某个状态
        deleteOneMdTdState ({ commit },  id) {
            commit('DELETE_ONE_MD_TD_STATE',  id)
        },

        setAccountsAsset ({ commit }, accountsAsset) {
            commit('SET_ACCOUNTS_ASSET', accountsAsset)
        },

        setTdList ({ commit }, tdList) {
            commit('SET_TD_LIST', tdList)
        },

        setMdList ({ commit }, mdList) {
            commit('SET_MD_LIST', mdList)
        },

        getTdMdList ({ dispatch }) {
            return Promise.all([getTdList(), getMdList()])
                .then(tdMdList => {
                    const tdList = tdMdList[0] || [];
                    const mdList = tdMdList[1] || [];
                    dispatch('setTdList', tdList);
                    dispatch('setMdList', mdList || [])

                    return {
                        tdList,
                        mdList
                    }
                })
            
        },

        setCurrentAccountTabName ({ commit }, name) {
            commit('SET_CURRENT_ACCOUNT_TAB_NAME', name)
        }
    },

    mutations: {

        SET_CURRENT_TICKER (state, ticker) {
            state.currentTicker = ticker;
        },

        SET_CURRENT_ACCOUNT_TAB_NAME (state, name) {
            state.currentAccountTabName = name;
        },

        SET_ONE_MD_TD_STATE (state, { id, stateData }) {
            Vue.set(state.mdTdState, id, Object.freeze(stateData))
        },
         
        DELETE_ONE_MD_TD_STATE (state, id) {
            delete state.mdTdState[id]
            Vue.delete(state.mdTdState, id)    
        },
        
        SET_ACCOUNTS_ASSET (state, accountsAsset) {
            state.accountsAsset = {
                ...state.accountsAsset,
                ...accountsAsset
            }
        },
        
        SET_ACCOUNT_ASSET (state, { accountId, accountsAsset }) {
            Vue.set(state.accountsAsset, accountId, accountsAsset)
        },
        
        SET_TD_LIST (state, tdList) {
            state.tdList = tdList
        },
        
        SET_MD_LIST (state, mdList) {
            state.mdList = mdList
        },
        
        SET_CURRENT_ACCOUNT (state, account) {
            state.currentAccount = account;
        },
    },

    getters: {}
}