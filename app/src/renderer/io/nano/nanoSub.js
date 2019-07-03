import { buildSubNmsg } from '@/io/nano/buildNmsg';
import { Observable } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import * as MSG_TYPE from '@/io/nano/msgType';

export const subGateWayState = () => {
    return new Observable(subscriber => {
        buildSubNmsg().on('data', buf => {
            const data = JSON.parse(String(buf).replace(/\0/g,'')) 
            subscriber.next(data)
        })
    })    
}

export const subObservable = subGateWayState()

//gateway state
export const buildGatewayStatePipe = () => {
    return subObservable.pipe(
        filter(d => d.msg_type === MSG_TYPE.gatewayState),
        map(({ data }) => {
            const category = data.category;
            const group = data.group;
            const name = data.name;
            if(+category === 0) return [`${MSG_TYPE.category[category]}_${group}`, data];
            else if(+category === 1) return [`${MSG_TYPE.category[category]}_${group}_${name}`, data];
        })
    )
}


//trading data: order trades position pnlmin
export const buildOrdersPipe = function(){
    const tradingDataTypes = [MSG_TYPE.order, MSG_TYPE.trade, MSG_TYPE.position, MSG_TYPE.portfolioByMin]
    return subObservable.pipe(
        filter(d => (tradingDataTypes.indexOf(d.msg_type) !== -1))
    )
}

//trades
export const buildTradesPipe = function(){
    const t = this;
    t.type = '';
    t.id = '';
    t.filter = (type, id) => {
        t.type = type;
        t.id = id; 
    }
    return subObservable.pipe(
        filter(d => d.msg_type === MSG_TYPE.trade),
        filter(d => {
            if(t.type === 'strategy') return d.strategy_id === t.id;
            else if(t.type === 'account') return d.account_id === t.id;
        })
    )
}




 

