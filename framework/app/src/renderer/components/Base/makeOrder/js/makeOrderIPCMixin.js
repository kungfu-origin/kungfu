
import { mapState, mapGetters } from 'vuex';

import { ipcEmitDataByName } from '__renderer/ipcMsg/emitter';
import { ipcRenderer } from 'electron';


export default {

    computed: {
        ...mapState({
            tdAccountSource: state => state.tdAccountSource || {},
            strategyList: state => state.strategyList,
            tdList: state => state.tdList,
            accountsAsset: state => state.accountsAsset,
        }),
    },

    methods: {
        cancelOrder (moduleType, orderData, strategyId) {
            return ipcEmitDataByName('cancelOrder', { moduleType, orderData, strategyId })
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            return ipcEmitDataByName('makeOrder', { moduleType, makeOrderForm, currentAccountResolved, strategyId })
        },

        subscribeTicker (sourceName, exchangeId, ticker) {
            return Promise.resolve(true)
        },
    }
}