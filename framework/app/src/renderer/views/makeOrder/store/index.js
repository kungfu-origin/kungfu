import Vue from 'vue'
import Vuex from 'vuex'

import { getAccountSource } from '__gConfig/accountConfig';

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
        //common
        setAccountsAsset ({ commit }, accountsAsset) {
            commit('SET_ACCOUNTS_ASSET', accountsAsset)
        },        

        //common
        setProcessStatus ({ commit }, processStatus) {
            commit('SET_PROCESS_STATUS', processStatus)
        },

        //common
        setTdAccountSource ({ commit }, tdAccountSource) {
            commit('SET_TD_ACCOUNT_SOURCE', tdAccountSource)
        },
        
        //common
        setMdAccountSource ({ commit }, mdAccountSource) {
            commit('SET_MD_ACCOUNT_SOURCE', mdAccountSource)
        },

        //common
        getAccountSourceConfig ({ dispatch }) {
            return getAccountSource()
            .then(accountSource => {
                dispatch('setTdAccountSource', accountSource.td)
                dispatch('setMdAccountSource', accountSource.md)
                return accountSource
            })
        },

        //common
        setStrategyList ({ commit }, strategyList) {
            commit('SET_STRATEGY_LIST', strategyList)
        },

        //common
        setTdList ({ commit }, tdList) {
            commit('SET_TD_LIST', tdList)
        },

        //common
        setMdList ({ commit }, mdList) {
            commit('SET_MD_LIST', mdList)
        }
    },

    getters: {
        
        proMode () {
            return false;
        }
    }
})