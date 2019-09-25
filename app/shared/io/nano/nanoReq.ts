import { buildRepNmsg } from '__io/nano/buildNmsg'
import * as msgType from '__io/nano/msgType'
const isEnglish = process.env.LANG_ENV === 'en';

interface RepData {
    msg_type: number,
    status: number,
    data: any
}

const buildRequest = (reqMsg: string, msgTypeVal: number, errMsg: string): Promise<any> => {
    return new Promise((resolve, reject) => {
        const req = buildRepNmsg();
        req.send(reqMsg)
        req.on('data', (buf: string) => {
            req.close();
            const data: RepData = JSON.parse(String(buf));
            if(data.msg_type === msgTypeVal ) {
                if(data.status === 200) {
                    resolve(data.data || {})
                } else {
                    reject(new Error(errMsg || ''))
                }
            }
        })
    })
}

export const nanoReqGatewayState = () : Promise<any> => {
    const reqMsg = JSON.stringify({
        msg_type: msgType.reqGatewayState,
        data: {}
    })
    return buildRequest(reqMsg, msgType.reqGatewayState, '请求柜台状态失败！')
}

export const nanoReqCash = () : Promise<any> => {
    const reqMsg = JSON.stringify({
        msg_type: msgType.reqCash,
        data: {}
    })
    return buildRequest(reqMsg, msgType.reqCash, '请求资金信息失败！')

}

//日历
//主动获得交易日
export const nanoReqCalendar = () : Promise<any> => {
    const reqMsg = JSON.stringify({
        msg_type: msgType.calendar,
        data: {}
    });
    return buildRequest(reqMsg, msgType.calendar, isEnglish ? 'Req tradingDay failed' : '请求交易日失败！') 
}

//请求删除策略持仓
export const nanoReqRemoveStrategyPos = (strategyId: string) : Promise<any> => {
    const reqMsg = JSON.stringify({
        msg_type: msgType.reqRemoveStrategyPos,
        data: {
            "name": strategyId,
            "mode": "live",
            "category": "strategy",
            "group": "default"
        }
    })
    // console.log('[REQ DELETE POS]', reqMsg)
    return buildRequest(reqMsg, msgType.reqRemoveStrategyPos, isEnglish ? 'Req delete strategy position failed' : '删除策略持仓失败！') 
}

//撤单
interface MakeOrderParams {
    accountId: string,
    orderId: string
}
export const nanoCancelOrder = ({ accountId, orderId }: MakeOrderParams) : Promise<any> => {
    const { source, id } = accountId.parseSourceAccountId();
    const reqMsg = JSON.stringify({
        msg_type: msgType.cancelOrder, 
        data: {
            'mode': 'live',
            'category': 'td',
            'group': source,
            'name': id,
            'order_id': orderId
        }
    })
    // console.log('[REQ CANCEL ORDER]', reqMsg)
    return buildRequest(reqMsg, msgType.cancelOrder, '撤单失败！') 
}

/** 全部撤单
 * @param  {String} {targetId gatewayname / strategyId
 * @param  {String} cancelType account / strategy
 * @param  {String} id strategyId / accountId}
 */
interface CancelAllOrderParams {
    cancelType: string,
    id: string
}
export const nanoCancelAllOrder = ({ cancelType, id }: CancelAllOrderParams): Promise<any> => {
        const reqMsg = JSON.stringify({
            'msg_type': msgType.cancelAllOrder,
            'data': {
                'mode': 'live',
                ...buildCancalAllOrderPostData(cancelType, id)
            }
        })
        return buildRequest(reqMsg, msgType.cancelAllOrder, isEnglish ? 'Cancel orders failed!' : '全部撤单失败！') 
}

function buildCancalAllOrderPostData(type: string, accountOrStrategyId: string) {
    if(type === 'account') {
        const { source, id } = accountOrStrategyId.parseSourceAccountId();
        return {
            category: 'td',
            group: source,
            name: id,
        }
    } else {
        return {
            category: 'strategy',
            group: 'default',
            name: accountOrStrategyId
        }
    }
}



interface MakeOrderData {
    account_id: string,
    price_type: string | number,
    side: string,
    offset: string
}

export const nanoMakeOrder = (makeOrderData: MakeOrderData) => {
    const reqMsg = JSON.stringify({
        req: 303,
        data: makeOrderData
    })
    return buildRequest(reqMsg, msgType.makeOrder, '下单失败！') 
}

