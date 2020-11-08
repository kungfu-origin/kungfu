import * as STRATEGY_API from '__io/kungfu/strategy';
import { deleteStrat } from '__io/actions/strategy';

//设置当前策略
export const setCurrentStrategy = ({commit}, strategy) => {
    commit('SET_CURRENT_STRATEGY', strategy)
}

//设置当前策略列表
export const setStrategyList = ({commit}, strategyList) => {
    commit('SET_STRATEGY_LIST', strategyList)
}

//删除策略
export const deleteStrategy = ({dispatch}, strategyId) => {
    return deleteStrat(strategyId).then(() => dispatch('getStrategyList'))
}

//更新straetegyList
export const getStrategyList = ({dispatch}) => {
    return STRATEGY_API.getStrategyList()
        .then((res) => {
            dispatch('setStrategyList', res);
            return res
        })
}

export const setStrategiesAsset = ({ commit }, strategiesAsset) => {
    commit('SET_STRATEGIES_ASSET', strategiesAsset)
}