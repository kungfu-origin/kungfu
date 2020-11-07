
import { ipcEmitDataByName } from '@/ipcMsg/emitter';

export default {

    methods: {
        cancelOrder (moduleType, orderData, strategyId) {
            return ipcEmitDataByName('cancelOrder', { moduleType, orderData, strategyId })
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            return ipcEmitDataByName('makeOrder', { moduleType, makeOrderForm, currentAccountResolved, strategyId })
        }
    }
}