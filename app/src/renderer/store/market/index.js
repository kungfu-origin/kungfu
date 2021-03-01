export default {

    state: {
        tickerSets: [],
    },

    actions: {
        getTickerSets ({ commit }, tickerSet) {
            commit('GET_TICKER_SETS', tickerSet)
        }
    },

    mutations: {
        GET_TICKER_SETS (state, tickerSet) {
            state.tickerSets = tickerSet
        }

    }
}