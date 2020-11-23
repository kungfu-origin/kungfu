export default {
    state: {
        currentDataSeriesId: ''
    },

    actions: {
        setCurrentDataSeriesId ({ commit }, dataSeriesId) {
            commit('SET_CURRENT_DATA_SERIES_ID', dataSeriesId)
        }
    },

    mutations: {
        SET_CURRENT_DATA_SERIES_ID (state, dataSeriesId) {
            state.currentDataSeriesId = dataSeriesId;
        }
    },

    getters: {}
}
