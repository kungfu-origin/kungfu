import { longfist, encodeKungfuLocation } from '__gUtils/kungfuUtils';
import { watcher, decodeKungfuLocation } from '__io/kungfu/watcher';

interface MakeOrderData {
    intrument_id: string,
    intrument_type: Number,
    exchange_id: string,
    limit_price: number,
    volume: number,
    price_type: number,
    side: number,
    offset: number,
    hedge_flag: number
}

export const kungfuMakeOrder = (makeOrderData: MakeOrderData, accountId: string, strategyId?: string) => {
    const accountLocation = encodeKungfuLocation(accountId, 'td');

    if (!watcher.isReadyToInteract(accountLocation)) {
        return Promise.reject(new Error(`需要先启动 TD ${accountId} 交易进程！`))
    }

    const orderInput = {
        ...longfist.OrderInput(),
        ...makeOrderData
    }

    if (strategyId) {
        const strategyLocation = encodeKungfuLocation(strategyId, 'strategy');
        return Promise.resolve(watcher.issueOrder(orderInput, accountLocation, strategyLocation))
    } else {
        return Promise.resolve(watcher.issueOrder(orderInput, accountLocation))
    }
}

export const kungfuCancelOrder = (orderId: string, accountId: string, strategyId?: string) => {
    const accountLocation = encodeKungfuLocation(accountId, 'td');
    if (!watcher.isReadyToInteract(accountLocation)) {
        return Promise.reject(new Error(`需要先启动 TD ${accountId} 交易进程！`))
    }

    const orderAction = {
        ...longfist.OrderAction(),
        order_id: BigInt(orderId)
    }

    if (strategyId) {
        const strategyLocation = encodeKungfuLocation(strategyId, 'strategy');
        return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation, strategyLocation))
    } else {
        return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation))
    }
}

export const kungfuCancelAllOrders = (orderDataList: OrderData[], strategyId?: string) => {
    const promiseList = orderDataList.map((orderData:  OrderData) => {
        const kungfuLocation = decodeKungfuLocation(orderData.source);
        const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
        const accountLocation = encodeKungfuLocation(accountId, 'td');

        const orderId = orderData.orderId;
        const orderAction = {
            ...longfist.OrderAction(),
            order_id: BigInt(orderId)
        }
    
        if (strategyId) {
            const strategyLocation = encodeKungfuLocation(strategyId, 'strategy');
            return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation, strategyLocation))
        } else {
            return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation))
        }
    })

    return Promise.all(promiseList)
}
