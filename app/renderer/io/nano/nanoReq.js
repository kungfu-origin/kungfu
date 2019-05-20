
import {reqCalendarNanomsg, reqOrderOperNanomsg} from '@/io/nano/buildNmsg'
import * as msgType from '@/io/nano/msgType'


//日历
//主动获得交易日
export const nanoGetCalendar = () => {
    return new Promise((resolve) => {
        const reqMsg = JSON.stringify({"req": "current"});
        const req = reqCalendarNanomsg();
        req.send(reqMsg + '\0')
        req.on('data', buf => {
            req.close();
            const data = JSON.parse(String(buf).replace(/\0/g,''))
            if(msgType.reqCalendar === data.msg_type) {
                resolve(data.data)
            }
        })
    })
}


//撤单
export const nanoCancelOrder = ({gatewayName, orderId}) => {
    return new Promise((resolve) => {
        const reqMsg = JSON.stringify({req: 304, data: {
            order_id: [orderId]
        }})
        const req = reqOrderOperNanomsg(gatewayName)
        req.send(reqMsg + '\0')
        req.close()
        resolve(true)
        req.on('data', buf => {
            const data = JSON.parse(String(buf).replace(/\0/g,''))
            if(msgType.cancelOrder === data.msg_type){
                //todo
            }
        })
    })
}


//全部撤单
export const nanoCancelAllOrder = ({gatewayName, type, id}) => {
    return new Promise((resolve) => {
        const postData = type === 'account' ? {account_id: id} : {client_id: id}
        const reqMsg = JSON.stringify({req: 304, data: postData})
        const req = reqOrderOperNanomsg(gatewayName)
        req.send(reqMsg + '\0')
        req.close()
        resolve(true)
        req.on('data', buf => {
            const data = JSON.parse(String(buf).replace(/\0/g,''))
            if(msgType.cancelOrder === data.msg_type){
                //todo
            }
        })
    })
}

