
import {reqCalendarNanomsg, reqGatewayNanomsg, reqStrategyNanomsg} from '@/io/nano/buildNmsg'
import * as msgType from '@/io/nano/msgType'


//日历
//主动获得交易日
export const nanoGetCalendar = () => {
    return new Promise(resolve => {
        const reqMsg = JSON.stringify({
            request: "calendar/current"
        });
        const req = reqCalendarNanomsg();
        req.send(reqMsg)
        req.on('data', buf => {
            req.close();
            const data = JSON.parse(String(buf));
            if(msgType.reqCalendar === data.msg_type) resolve(data.data)
        })
    })
}

//撤单
export const nanoCancelOrder = ({gatewayName, orderId}) => {
    return new Promise(resolve => {
        const reqMsg = JSON.stringify({
            req: 304, 
            data: {order_id: [orderId]}
        })
        const req = reqGatewayNanomsg(gatewayName)
        req.send(reqMsg)
        req.on('data', buf => {
            req.close()
            const data = JSON.parse(String(buf))
            if(msgType.cancelOrder === data.msg_type) resolve(true)
        })
    })
}

/** 全部撤单
 * @param  {String} {targetId gatewayname / strategyId
 * @param  {String} cancelType account / strategy
 * @param  {String} id strategyId / accountId}
 */
export const nanoCancelAllOrder = ({targetId, cancelType, id}) => {
    return new Promise(resolve => {
        const postData = cancelType === 'account' ? {account_id: id} : {client_id: id}
        const reqMsg = JSON.stringify({
            req: 304, 
            data: postData
        })
        const req = cancelType ==='account' ? reqGatewayNanomsg(targetId) : reqStrategyNanomsg(targetId)
        req.send(reqMsg)
        req.on('data', buf => {
            req.close()
            const data = JSON.parse(String(buf))
            if(msgType.cancelOrder === data.msg_type) resolve(true)
        })
    })
}


export const nanoMakeOrder = (gatewayName, makeOrderData) => {
    return new Promise(resolve => {
        const reqMsg = JSON.stringify({
            req: 303,
            data: makeOrderData
        })
        const req = reqGatewayNanomsg(gatewayName)
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