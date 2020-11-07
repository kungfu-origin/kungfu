
import { kungfuCancelOrder, kungfuMakeOrder } from '__io/kungfu/makeCancelOrder';
import { decodeKungfuLocation } from '__io/kungfu/watcher';

export default {

    methods: {
        cancelOrder (moduleType, orderData, strategyId) {
            const kungfuLocation = decodeKungfuLocation(orderData.source);
            const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
            
            //撤单   
            if (moduleType === 'strategy') {
                return kungfuCancelOrder( orderData.orderId, accountId, strategyId)
            } else if (moduleType === 'account') {
                return kungfuCancelOrder( orderData.orderId, accountId)
            }
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            if (moduleType === 'account') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved)
            } else if (moduleType === 'strategy') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved, strategyId)
            }
        },
    }

}