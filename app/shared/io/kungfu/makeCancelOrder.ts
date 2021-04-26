import { longfist, encodeKungfuLocation } from '__io/kungfu/kungfuUtils';
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

export const kungfuSubscribeTicker = (sourceName: string, exchangeId: string, ticker: string) => {
    return Promise.resolve(
        watcher.requestMarketData(
            encodeKungfuLocation(sourceName, 'md'),
            exchangeId, 
            ticker
        )
    )
}

export const kungfuMakeOrder = (makeOrderData: MakeOrderData, accountId: string, strategyId?: string, parentId?: number) => {
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
        //设置orderInput的parentid，来标记该order为策略手动下单
        const parentId = BigInt(+new Date().getTime());

        return Promise.resolve(watcher.issueOrder({
            ...orderInput,
            parent_id: parentId
        }, accountLocation, strategyLocation))
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
