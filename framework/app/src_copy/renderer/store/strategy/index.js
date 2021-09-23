import { getStrategyList } from '@kungfu-trader/kungfu-uicc/io/kungfu/strategy';
import { deleteStrat } from '@kungfu-trader/kungfu-uicc/io/actions/strategy';

export default {

    state: {
        currentStrategy: '', //当前运行策略
        strategyList: [], //策略列表
        strategyiesAsset: {}
    },

    actions: {
        //设置当前策略
        setCurrentStrategy ({commit}, strategy) {
            commit('SET_CURRENT_STRATEGY', strategy)
        },

        //设置当前策略列表
        setStrategyList ({commit}, strategyList) {
            commit('SET_STRATEGY_LIST', strategyList)
        },

        //删除策略
        deleteStrategy ({dispatch}, strategyId) {
            return deleteStrat(strategyId).then(() => dispatch('getStrategyList'))
        },

        //更新straetegyList
        getStrategyList ({dispatch}) {
            return getStrategyList()
                .then((res) => {
                    dispatch('setStrategyList', res);
                    return res
                })
        },

        setStrategiesAsset ({ commit }, strategiesAsset) {
            commit('SET_STRATEGIES_ASSET', strategiesAsset)
        },
    },

    mutations: {
        SET_CURRENT_STRATEGY (state, strategy) {
            state.currentStrategy = strategy;
        },
        
        SET_STRATEGY_LIST (state, strategyList) {
            state.strategyList = strategyList
        },
        
        SET_STRATEGIES_ASSET (state, strategiesAssets) {
            state.strategyiesAsset = strategiesAssets;
        },
    },

    getters: {}
}
