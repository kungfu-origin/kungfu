import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

export default new Vuex.Store({

	state: {
        tdList: [],
        mdList: [],
        strategyList: [],
        tdAccountSource: {},
        mdAccountSource: {},
        processStatus: {},
        accountsAsset: {}
    },

    mutations: {

        SET_TD_LIST (state, tdList) {
            state.tdList = tdList
        },
        
        SET_MD_LIST (state, mdList) {
            state.mdList = mdList
        },

        SET_STRATEGY_LIST (state, strategyList) {
            state.strategyList = strategyList
        },

        SET_TD_ACCOUNT_SOURCE (state, tdAccountSource) {
            state.tdAccountSource = tdAccountSource
        },
        
        SET_MD_ACCOUNT_SOURCE (state, mdAccountSource) {
            state.mdAccountSource = mdAccountSource
        },

        SET_PROCESS_STATUS (state, processStatus) {
            state.processStatus = processStatus
        },

        SET_ACCOUNTS_ASSET (state, accountsAsset) {
            state.accountsAsset = {
                ...state.accountsAsset,
                ...accountsAsset
            }
        }
    },

    actions: {
        setAccountsAsset ({ commit }, accountsAsset) {
            commit('SET_ACCOUNTS_ASSET', accountsAsset)
        },        

        setProcessStatus ({ commit }, processStatus) {
            commit('SET_PROCESS_STATUS', processStatus)
        },

        setTdAccountSource ({ commit }, tdAccountSource) {
            commit('SET_TD_ACCOUNT_SOURCE', tdAccountSource)
        },
        
        setMdAccountSource ({ commit }, mdAccountSource) {
            commit('SET_MD_ACCOUNT_SOURCE', mdAccountSource)
        },

        //设置当前策略列表
        setStrategyList ({ commit }, strategyList) {
            commit('SET_STRATEGY_LIST', strategyList)
        },

        setTdList ({ commit }, tdList) {
            commit('SET_TD_LIST', tdList)
        },

        setMdList ({ commit }, mdList) {
            commit('SET_MD_LIST', mdList)
        }
    }
})