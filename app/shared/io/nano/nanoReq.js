
import { buildRepNmsg, buildWatcherRepNmsg } from '__io/nano/buildNmsg'
import * as msgType from '__io/nano/msgType'

export const nanoReqGatewayState = () => {
    return new Promise((resolve, reject) => {
        const reqMsg = JSON.stringify({
            msg_type: msgType.reqGatewayState,
            data: {}
        })
        const req = buildWatcherRepNmsg();
        req.send(reqMsg)
        req.on('data', buf => {
            req.close();
            const data = JSON.parse(String(buf));
            if(data.msg_type === msgType.reqGatewayState ) {
                if(data.status === 200) {
                    resolve(data)
                } else {
                    reject(new Error('请求gatewayState失败！'))
                }
            }
        })
    })
}

//日历
//主动获得交易日
export const nanoReqCalendar = () => {
    return new Promise((resolve, reject) => {
        const reqMsg = JSON.stringify({
            msg_type: msgType.calendar,
            data: {}
        });
        const req = buildRepNmsg();
        req.send(reqMsg)
        req.on('data', buf => {
            req.close();
            const data = JSON.parse(String(buf));
            if(data.msg_type === msgType.calendar ) {
                if(data.status === 200) {
                    resolve(data.data)
                } else {
                    reject(new Error('请求交易日失败！'))
                }
            }
        })
    })
}

//撤单
export const nanoCancelOrder = ({ sourceAccountId, orderId }) => {
    return new Promise((resolve, reject) => {
        const { source, id } = sourceAccountId.parseSourceAccountId();
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
        const req = buildRepNmsg()
        req.send(reqMsg)
        req.on('data', buf => {
            req.close()
            const data = JSON.parse(String(buf))
            if(data.msg_type === msgType.cancelOrder) {
                if(data.status === 200) {
                    resolve(data)
                } else {
                    reject(new Error(''))
                } 
            }
        })
    })
}

/** 全部撤单
 * @param  {String} {targetId gatewayname / strategyId
 * @param  {String} cancelType account / strategy
 * @param  {String} id strategyId / accountId}
 */
export const nanoCancelAllOrder = ({ cancelType, id }) => {
    return new Promise(resolve => {
        const reqMsg = JSON.stringify({
            'msg_type': msgType.cancelAllOrder,
            'data': {
                'mode': 'live',
                ...buildCancalAllOrderPostData(cancelType, id)
            }
        })
        const req = buildRepNmsg();
        req.send(reqMsg)
        req.on('data', buf => {
            req.close()
            const data = JSON.parse(String(buf))
            if(data.msg_type === msgType.cancelAllOrder) {
                if(data.status === 200) {
                    resolve(data)
                } else {
                    reject(new Error(''))
                } 
            }
        })
    })
}

function buildCancalAllOrderPostData(type, accountOrStrategyId) {
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

export const nanoMakeOrder = (gatewayName, makeOrderData) => {
    return new Promise(resolve => {
        const reqMsg = JSON.stringify({
            req: 303,
            data: makeOrderData
        })
        const req = buildRepNmsg(gatewayName)
        req.send(reqMsg)
        req.on('data', buf => {
            req.close()
            const data = JSON.parse(String(buf))
            if(msgType.makeOrder === data.msg_type){
                resolve(true)
            }
        })
    })
}

