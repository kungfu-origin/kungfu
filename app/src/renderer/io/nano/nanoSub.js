import { buildSubNmsg } from '@/io/nano/buildNmsg';
import { Observable } from 'rxjs';
import * as MSG_TYPE from '@/io/nano/msgType';

export const subGateWayState = () => {
    const observable = new Observable((subscriber) => {
        buildSubNmsg().on('data', buf => {
            const data = JSON.parse(String(buf).replace(/\0/g,'')) 
            subscriber.next(data)
        })
    })
    return observable
    
}


export const subObservable = subGateWayState()

export const filterGatewayState = (data) => {
    return new Promise(resolve => {
        const msgType = data.msg_type;
        if(msgType !== MSG_TYPE.gatewayState) return;
    })
   

}
