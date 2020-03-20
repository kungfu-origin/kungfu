import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { dealOrder, dealTrade, dealPos, dealAsset } from '__io/kungfu/watcher';

import { map } from 'rxjs/operators';


// orders: transformOrderTradeListToData(orders, type),
// trades: transformOrderTradeListToData(trades, type),
// positions: transformTradingItemListToData(positions, type),
// assets: transformAssetItemListToData(assets, type),
// pnl: transformTradingItemListToData(pnl, type),
// orderStat: transformOrderStatListToData(orderStat)


export const tradingDataObservale = (type: string, processId: string) => {
    return buildTradingDataPipe(type).pipe(
        map((data: any) => {

            const orders = data.orders[processId] || [];
            const ordersResolve = dealOrdersFromWatcher(orders);

            const trades = data.trades[processId] || [];
            const tradesResolve = dealTradesFromWathcer(trades);

            const positions = data.positions[processId] || [];
            const positionsResolve = dealPosFromWatcher(positions);

            const assetsResolve = dealAsset(data.assets[processId] || []);

            return {
                orders: ordersResolve,
                trades: tradesResolve,
                positions: positionsResolve,
                assets: assetsResolve
            }
        })
    )
}


function dealOrdersFromWatcher (orders: OrderInputData[]) {
    let orderDataByKey: { [propName: string]: OrderData } = {};

    orders.kfForEach((item: OrderInputData) => {
        let orderData = dealOrder(item);
        orderDataByKey[orderData.id] = orderData;
    })

    return Object.freeze(Object.values(orderDataByKey).sort((a: OrderData, b: OrderData) =>{
        return  b.updateTimeNum - a.updateTimeNum
    }))
}

function dealTradesFromWathcer (trades: TradeInputData[]) {
    return trades
        .map(item => dealTrade(item))
        .sort((a: TradeData, b: TradeData) => (b.updateTimeNum - a.updateTimeNum))
}

function dealPosFromWatcher (positions: PosInputData[]) {
    let positionDataByKey: { [propName: string]: PosData } = {};
    
    positions.kfForEach((item: PosInputData) => {
        let positionData = dealPos(item);
        const poskey = positionData.instrumentId + positionData.direction
        positionDataByKey[poskey] = positionData;
    })

    return positionDataByKey
}