import Vue from 'vue';
import {deepClone} from '__gUtils/busiUtils'

export const SET_CURRENT_STRATEGY = (state, strategy) => {
    state.currentStrategy = strategy;
}

export const SET_STRATEGY_LIST = (state, strategyList) => {
    state.strategyList = strategyList
}

export const SET_STRATEGIES_ASSET = (state, strategiesAssets) => {
    state.strategyiesAsset = strategiesAssets;
}


