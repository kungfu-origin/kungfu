/*
 * @Author: your name
 * @Date: 2020-05-22 11:55:44
 * @LastEditTime: 2020-05-25 12:46:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/assets/scripts/actions/tradingDataActions.ts
 */ 
import { map } from 'rxjs/operators';
import { buildKungfuDataByAppPipe } from '__io/kungfu/tradingData';
import { watcher, transformTradingItemListToData, transformAssetItemListToData, getOrdersBySourceDestInstrumentId, getTradesBySourceDestInstrumentId, getOrderStatByDest, transformOrderStatListToData, dealPos, dealAsset, dealOrderStat } from '__io/kungfu/watcher';
import { encodeKungfuLocation } from '__io/kungfu/kungfuUtils';
import { ensureLedgerData } from '__gUtils/busiUtils';

export const tradingDataObservale = (type: string, processId: string) => {
    const sourceDest = getLocationUID(type, processId) 
    return buildKungfuDataByAppPipe().pipe(
        map(() => {
            const ledgerData = watcher.ledger;

            const positions = ensureLedgerData(ledgerData.Position).map((item: PosOriginData) => dealPos(item));
            const assets = ensureLedgerData(ledgerData.Asset).map((item: AssetOriginData) => dealAsset(item));

            if (type === 'account') {
                const orders = getOrdersBySourceDestInstrumentId(ledgerData.Order, 'source', sourceDest);
                const trades = getTradesBySourceDestInstrumentId(ledgerData.Trade, 'source', sourceDest);
                const orderStat = getOrderStatByDest(ledgerData.OrderStat, 'dest', sourceDest);
                const orderStatResolved = transformOrderStatListToData(orderStat);  
                const ordersResolved = dealOrdersFromWatcher(orders, orderStatResolved);
                const tradesResolved = dealTradesFromWathcer(trades);
                const positionsResolved = transformTradingItemListToData(positions, 'account')[processId] || [];
                const assetsResolved = transformAssetItemListToData(assets, 'account')[processId] || [];


                return {
                    orders: ordersResolved,
                    trades: tradesResolved,
                    positions: dealPosFromWatcher(positionsResolved),
                    assets: assetsResolved
                }

            } else if (type === 'strategy') {
                const orders = getOrdersBySourceDestInstrumentId(ledgerData.Order, 'dest', sourceDest);
                const trades = getTradesBySourceDestInstrumentId(ledgerData.Trade, 'dest', sourceDest);
                const orderStat = getOrderStatByDest(ledgerData.OrderStat);
                const orderStatResolved = transformOrderStatListToData(orderStat);  
                const ordersResolved = dealOrdersFromWatcher(orders, orderStatResolved);
                const tradesResolved = dealTradesFromWathcer(trades);
                const positionsResolved = transformTradingItemListToData(positions, 'strategy')[processId] || [];
                const assetsResolved = transformAssetItemListToData(assets, 'strategy')[processId] || [];

                return {
                    orders: ordersResolved,
                    trades: tradesResolved,
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


function getLocationUID (type: string, currentId: string) {
    if (type === 'account') {
        return watcher.getLocationUID(encodeKungfuLocation(currentId, 'td'));
    } else if (type === 'strategy') {
        return watcher.getLocationUID(encodeKungfuLocation(currentId, 'strategy'));
    } else {
        console.error('getLocationUID type is not account or strategy')
        return 0
    }
}

function dealOrdersFromWatcher (orders: OrderOriginData[], orderStatOrigin: { [prop: string]: OrderStatOriginData }) {
    let orderDataByKey: { [propName: string]: OrderData } = {};
    orders.kfForEach((orderData: OrderData) => {
        const latencyData: any = orderStatOrigin[orderData.orderId] || null;
        const orderStat = dealOrderStat(latencyData)

        //@ts-ignore
        const latencySystem = orderStat.latencySystem || '';
        //@ts-ignore
        const latencyNetwork = orderStat.latencyNetwork || '';

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