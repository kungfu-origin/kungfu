/*
 * @Author: your name
 * @Date: 2020-05-22 11:55:44
 * @LastEditTime: 2020-05-25 12:46:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/assets/scripts/actions/tradingDataActions.ts
 */ 
import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { watcher, transformOrderStatListToData } from '__io/kungfu/watcher';
import { map } from 'rxjs/operators';
import { ensureLedgerData } from '__gUtils/busiUtils';


export const tradingDataObservale = (type: string, processId: string) => {
    return buildTradingDataPipe(type).pipe(
        map((data: any) => {

            const orders = data.orders[processId] || [];
            const orderStat = ensureLedgerData(watcher.ledger.OrderStat, 'insert_time').slice(0, 1000);
            const orderStatResolved = transformOrderStatListToData(orderStat);  

            const ordersResolved = dealOrdersFromWatcher(orders, orderStatResolved);

            const trades = data.trades[processId] || [];
            const tradesResolve = dealTradesFromWathcer(trades);

            const positions = data.positions[processId] || [];
            const positionsResolve = dealPosFromWatcher(positions);

            const assetsResolve = data.assets[processId] || [];

            return {
                orders: ordersResolved,
                trades: tradesResolve,
                positions: positionsResolve,
                assets: assetsResolve
            }
        })
    )
}


function dealOrdersFromWatcher (orders: OrderOriginData[], orderStat: { [prop: string]: OrderStatData }) {
    let orderDataByKey: { [propName: string]: OrderData } = {};
    orders.kfForEach((item: OrderData) => {
        const orderData = item;
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

function dealTradesFromWathcer (trades: TradeData[]) {
    return trades.sort((a: TradeData, b: TradeData) => b.updateTimeNum - a.updateTimeNum)
}

function dealPosFromWatcher (positions: PosData[]) {
    let positionDataByKey: { [propName: string]: PosData } = {};
    
    positions
    .sort((pos1: PosData, pos2: PosData) => {
        if (pos1.instrumentId > pos2.instrumentId) {
            return 1
        } else if (pos1.instrumentId < pos2.instrumentId) {
            return -1
        } else {
            return 0
        };
    })
    .kfForEach((item: PosData) => {
        let positionData = item;
        const poskey = positionData.instrumentId + positionData.direction
        positionDataByKey[poskey] = positionData;
    })

    return positionDataByKey
}