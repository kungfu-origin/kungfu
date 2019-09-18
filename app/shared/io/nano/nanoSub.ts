import { buildSubNmsg } from '__io/nano/buildNmsg';
import { Observable } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import * as MSG_TYPE from '__io/nano/msgType';

export const subGateWayState = () => {
    return new Observable(subscriber => {
        buildSubNmsg().on('data', (buf: string) => {
            const data = JSON.parse(String(buf).replace(/\0/g,'')) 
            subscriber.next(data)
        })
    })    
}

export const subObservable = subGateWayState()

//gateway state
export const buildGatewayStatePipe = () => {
    return subObservable.pipe(
        filter((d: any) => d.msg_type === MSG_TYPE.gatewayState),
        map(({ data }: any) => {
            const { category, group, name }: any = data;
            if(+category === 0) return [`${MSG_TYPE.category[category]}_${group}`, data];
            else if(+category === 1) return [`${MSG_TYPE.category[category]}_${group}_${name}`, data];
        })
    )
}

//trading data: order trades position pnlmin
export const buildTradingDataPipe = () => {
    const tradingDataTypes = [MSG_TYPE.order, MSG_TYPE.trade, MSG_TYPE.position, MSG_TYPE.portfolio]
    return subObservable.pipe(
        filter((d: any) => (tradingDataTypes.indexOf(d.msg_type) !== -1))
    )
}

//cash
export const buildCashPipe = () => {
    return subObservable.pipe(
        filter((d: any) => d.msg_type === MSG_TYPE.accountInfo)
    )
}

//tradingDay
export const buildTradingDayPipe = () => {
    return subObservable.pipe(
        filter((d: any) => d.msg_type === MSG_TYPE.calendar)
    )
}


 

