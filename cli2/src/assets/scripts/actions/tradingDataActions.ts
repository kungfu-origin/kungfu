/*
 * @Author: your name
 * @Date: 2020-05-22 11:55:44
 * @LastEditTime: 2020-05-25 12:46:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/assets/scripts/actions/tradingDataActions.ts
 */ 
import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { dealOrder, dealTrade, dealPos } from '__io/kungfu/watcher';
import { map } from 'rxjs/operators';


export const tradingDataObservale = (type: string, processId: string) => {
    return buildTradingDataPipe(type).pipe(
        map((data: any) => {

            const orders = data.orders[processId] || [];
            const orderStat = data.orderStat;
            const ordersResolve = dealOrdersFromWatcher(orders, orderStat);

            const trades = data.trades[processId] || [];
            const tradesResolve = dealTradesFromWathcer(trades);

            const positions = data.positions[processId] || [];
            const positionsResolve = dealPosFromWatcher(positions);

            const assetsResolve = data.assets[processId] || [];

            return {
                orders: ordersResolve,
                trades: tradesResolve,
                positions: positionsResolve,
                assets: assetsResolve
            }
        })
    )
}


function dealOrdersFromWatcher (orders: OrderOriginData[], orderStat: { [prop: string]: OrderStatData }) {
    let orderDataByKey: { [propName: string]: OrderData } = {};
    orders.kfForEach((item: OrderOriginData) => {
        const orderData = dealOrder(item);
        const latencyData = orderStat[orderData.orderId] || {};
        const latencySystem = latencyData.latencySystem || '';
        const latencyNetwork = latencyData.latencyNetwork || '';

        orderDataByKey[orderData.id] = {
            ...orderData,
            latencySystem,
            latencyNetwork
        };
    })

    return Object.freeze(Object.values(orderDataByKey).sort((a: OrderData, b: OrderData) =>{
        return  b.updateTimeNum - a.updateTimeNum
    }))
}

function dealTradesFromWathcer (trades: TradeOriginData[]) {
    return trades
        .map(item => dealTrade(item))
        .sort((a: TradeData, b: TradeData) => b.updateTimeNum - a.updateTimeNum)
}

function dealPosFromWatcher (positions: PosOriginData[]) {
    let positionDataByKey: { [propName: string]: PosData } = {};
    
    positions
    .sort((pos1: PosOriginData, pos2: PosOriginData) => {
        if (pos1.instrument_id > pos2.instrument_id) {
            return 1
        } else if (pos1.instrument_id < pos2.instrument_id) {
            return -1
        } else {
            return 0
        };
    })
    .kfForEach((item: PosOriginData) => {
        let positionData = dealPos(item);
        const poskey = positionData.instrumentId + positionData.direction
        positionDataByKey[poskey] = positionData;
    })

    return positionDataByKey
}