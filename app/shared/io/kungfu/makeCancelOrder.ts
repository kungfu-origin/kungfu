import { longfist, encodeKungfuLocation } from '__gUtils/kungfuUtils';
import { watcher } from '__io/kungfu/watcher';

interface MakeOrderData {
    intrument_id: string,
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
        return Promise.reject(new Error(`TD ${accountId} 异常，请稍后再试！`))
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

export const kungfuCancelOrder = (orderId: string, accountId: string, strategyId: string) => {
    const accountLocation = encodeKungfuLocation(accountId, 'td');
    if (!watcher.isReadyToInteract(accountLocation)) {
        return Promise.reject(new Error(`TD ${accountId} 异常，请稍后再试！`))
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

// /** 全部撤单
//  * @param  {String} {targetId gatewayname / strategyId
//  * @param  {String} cancelType account / strategy
//  * @param  {String} id strategyId / accountId}
//  */
// interface CancelAllOrderParams {
//     cancelType: string,
//     id: string
// }
// export const nanoCancelAllOrder = ({ cancelType, id }: CancelAllOrderParams): Promise<any> => {
//         const reqMsg = JSON.stringify({
//             'msg_type': msgType.cancelAllOrder,
//             'data': {
//                 'mode': 'live',
//                 ...buildCancalAllOrderPostData(cancelType, id)
//             }
//         })
//         return buildRequest(reqMsg, msgType.cancelAllOrder, isEnglish ? 'Cancel orders failed!' : '全部撤单失败！') 
// }

// function buildCancalAllOrderPostData(type: string, accountOrStrategyId: string) {
//     if(type === 'account') {
//         const { source, id } = accountOrStrategyId.parseSourceAccountId();
//         return {
//             category: 'td',
//             group: source,
//             name: id,
//         }
//     } else {
//         return {
//             category: 'strategy',
//             group: 'default',
//             name: accountOrStrategyId
//         }
//     }
// }

