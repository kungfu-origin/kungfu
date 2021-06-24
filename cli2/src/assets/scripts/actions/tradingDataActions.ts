/*
 * @Author: your name
 * @Date: 2020-05-22 11:55:44
 * @LastEditTime: 2020-05-25 12:46:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/assets/scripts/actions/tradingDataActions.ts
 */ 
import { map } from 'rxjs/operators';
import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { watcher, getOrdersBySourceDestInstrumentId, getTradesBySourceDestInstrumentId, getOrderStatByDest, transformOrderStatListToData } from '__io/kungfu/watcher';
import { encodeKungfuLocation } from '__io/kungfu/kungfuUtils';


export const tradingDataObservale = (type: string, processId: string) => {
    const sourceDest = getLocationUId(type, processId) 
    return buildTradingDataPipe(type).pipe(
        map((data: any) => {
            const ledgerData = watcher.ledger;

            const positions = data.positions[processId] || [];
            const positionsResolved = dealPosFromWatcher(positions);
            const assetsResolved = data.assets[processId] || [];


            if (type === 'account') {
                const orders = getOrdersBySourceDestInstrumentId(ledgerData.Order, 'source', sourceDest);
                const trades = getTradesBySourceDestInstrumentId(ledgerData.Trade, 'source', sourceDest);
                const orderStat = getOrderStatByDest(ledgerData.OrderStat, 'dest', sourceDest);
                const orderStatResolved = transformOrderStatListToData(orderStat);  
                const ordersResolved = dealOrdersFromWatcher(orders, orderStatResolved);
                const tradesResolve = dealTradesFromWathcer(trades);


                return {
                    orders: ordersResolved,
                    trades: tradesResolve,
                    positions: positionsResolved,
                    assets: assetsResolved
                }
            } else if (type === 'strategy') {
                const orders = getOrdersBySourceDestInstrumentId(ledgerData.Order, 'dest', sourceDest);
                const trades = getTradesBySourceDestInstrumentId(ledgerData.Trade, 'dest', sourceDest);
                const orderStat = getOrderStatByDest(ledgerData.OrderStat);
                const orderStatResolved = transformOrderStatListToData(orderStat);  
                const ordersResolved = dealOrdersFromWatcher(orders, orderStatResolved);
                const tradesResolve = dealTradesFromWathcer(trades);


                return {
                    orders: ordersResolved,
                    trades: tradesResolve,
                    positions: positionsResolved,
                    assets: assetsResolved
                }
            } else {

                return {
                    orders: [],
                    trades: [],
                    positions: {},
                    assets: []
                }
            }
        })
    )
}


function getLocationUId (type: string, currentId: string) {
    if (type === 'account') {
        return watcher.getLocationUId(encodeKungfuLocation(currentId.toAccountId(), 'td'));
    } else if (type === 'strategy') {
        return watcher.getLocationUId(encodeKungfuLocation(currentId, 'strategy'));
    } else {
        console.error('getLocationUId type is not account or strategy')
        return 0
    }
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

function dealPosFromWatcher (positions: PosData[]): { [propName: string]: PosData } {
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