import { longfist, encodeKungfuLocation } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import { watcher, decodeKungfuLocation } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';

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

export const kungfuSubscribeInstrument = (sourceName: string, exchangeId: string, ticker: string) => {
    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接！`))
    }

    const mdLocation = encodeKungfuLocation(sourceName, 'md');
    
    if (!watcher.isReadyToInteract(mdLocation)) {
        if (process.env.NODE_ENV === 'development') {
            console.log(mdLocation, 'is not ready')
        }
        return Promise.resolve(false)
    }

    return Promise.resolve(
        watcher.requestMarketData(
            mdLocation,
            exchangeId, 
            ticker
        )
    )
}

export const kungfuMakeOrder = (makeOrderData: MakeOrderData, accountId: string, strategyId?: string, parentId?: number) => {

    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接！`))
    }

    const accountLocation = encodeKungfuLocation(accountId, 'td');
    if (!watcher.isReadyToInteract(accountLocation)) {
        return Promise.reject(new Error(`需要先启动 TD ${accountId} 交易进程！`))
    }

    const orderInput = {
        ...longfist.OrderInput(),
        ...makeOrderData,
        insert_time: watcher.now()
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
    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接！`))
    }

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

export const kungfuCancelAllOrders = (orderDataList: OrderOriginData[], strategyId?: string) => {
    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接！`))
    }
    
    const promiseList = orderDataList.map((orderData:  OrderOriginData) => {
        const kungfuLocation = decodeKungfuLocation(+orderData.source);
        const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
        const accountLocation = encodeKungfuLocation(accountId, 'td');

        if (!watcher.isReadyToInteract(accountLocation)) {
            return Promise.resolve(false)
        }

        const orderId = orderData.order_id;
        const orderAction = {
            ...longfist.OrderAction(),
            order_id: BigInt(orderId)
        }
    
        if (strategyId) {
            const strategyLocation = encodeKungfuLocation(strategyId, 'strategy');
            if (!watcher.isReadyToInteract(strategyLocation)) {
                return Promise.resolve(false)
            }
            return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation, strategyLocation))
        } else {
            return Promise.resolve(watcher.cancelOrder(orderAction, accountLocation))
        }
    })

    return Promise.all(promiseList)
}
