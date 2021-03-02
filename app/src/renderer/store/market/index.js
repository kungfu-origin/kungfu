export default {

    state: {
        tickerSets: [],

        currentTickerSet: null,

        currentTicker: null
    },

    actions: {
        setTickerSets ({ commit }, tickerSets) {
            commit('SET_TICKER_SETS', tickerSets)
        },

        setCurrentTickerSet ({ commit }, tickerSet) {
            commit('SET_CURRENT_TICKER_SET', tickerSet)
        }
    },

    mutations: {
        SET_TICKER_SETS (state, tickerSets) {
            state.tickerSets = tickerSets
        },

        SET_CURRENT_TICKER_SET (state, tickerSet) {
            state.currentTickerSet = tickerSet
        }

    }
}