import Vue from 'vue';

export default {

    state: {
        tickerSets: [],

        currentTickerSet: null,

        currentTicker: null,

        marketAvgVolume: {},

        quotes: [],
    },

    actions: {
        setTickerSets ({ commit }, tickerSets) {
            commit('SET_TICKER_SETS', tickerSets)
        },

        setCurrentTickerSet ({ commit }, tickerSet) {
            commit('SET_CURRENT_TICKER_SET', tickerSet)
        },

        setMarketAvgVolume ({ commit }, { days, data }) {
            commit('SET_MARKET_AVG_VOLUME', { days, data })
        },

        setQuotes ({ commit }, quotes) {
            commit('SET_QUOTES', quotes)
        }
    },

    mutations: {
        SET_QUOTES (state, quotes) {
            state.quotes = quotes;
        },

        SET_TICKER_SETS (state, tickerSets) {
            state.tickerSets = tickerSets
        },

        SET_CURRENT_TICKER_SET (state, tickerSet) {
            state.currentTickerSet = tickerSet
        },

        SET_MARKET_AVG_VOLUME (state, { days, data }) {
            Vue.set(state.marketAvgVolume, days, data)
        }

    },

    getters: {
        flatternTickers (state) {
            let tickersList = [];
            (state.tickerSets || []).forEach(tickerSet => {
                const tickers = tickerSet.tickers || [];
                tickersList = [ ...tickersList, ...tickers]
            });

            return tickersList || []
        }
    }
}