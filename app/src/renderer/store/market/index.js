export default {

    state: {
        tickerSets: [],
    },

    actions: {
        addTickerSets ({ commit }, tickerSet) {
            commit('ADD_TICKER_SETS', tickerSet)
        }
    },

    mutations: {
        ADD_TICKER_SETS (state, tickerSet) {
            state.tickerSets.push(tickerSet)
        }

    }
}