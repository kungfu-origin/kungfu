import { watcher, encodeKungfuLocation } from '__gUtils/kungfuUtils';


interface MakeOrderData {
    category: string | 'td',
    group: string, // source_name
    name: string, // account_id
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
    if (!watcher.isReadyToInteract(location)) {
        return Promise.reject(new Error(`TD ${accountId} 异常，请稍后再试！`))
    }

    const strategyLocation = strategyId ? encodeKungfuLocation(strategyId, 'strategy') : null;
    const res = watcher.issueOrder(makeOrderData, accountLocation, [ strategyLocation ])
    return Promise.resolve(res)
}


// //撤单
// interface MakeOrderParams {
//     accountId: string,
//     orderId: string
// }
// export const nanoCancelOrder = ({ accountId, orderId }: MakeOrderParams) : Promise<any> => {
//     const { source, id } = accountId.parseSourceAccountId();
//     const reqMsg = JSON.stringify({
//         msg_type: msgType.cancelOrder, 
//         data: {
//             'mode': 'live',
//             'category': 'td',
//             'group': source,
//             'name': id,
//             'order_id': orderId
//         }
//     })
//     // console.log('[REQ CANCEL ORDER]', reqMsg)
//     return buildRequest(reqMsg, msgType.cancelOrder, '撤单失败！') 
// }

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

// export const nanoMakeOrder = (makeOrderData: MakeOrderData) => {
//     const reqMsg = JSON.stringify({
//         msg_type: msgType.makeOrder,
//         data: {
//             mode: 'live',
//             ...makeOrderData
//         }
//     })
//     return buildRequest(reqMsg, msgType.makeOrder, '下单失败！') 
// }

