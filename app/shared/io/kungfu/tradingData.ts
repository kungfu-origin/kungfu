import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    dealGatewayStates, 
    transformTradingItemListToData, 
    transformOrderTradeListToData, 
    transformOrderStatListToData, 
    transformAssetItemListToData 
} from '__io/kungfu/watcher';

import { startGetKungfuTradingData, startGetKungfuGlobalData } from '__io/kungfu/watcher';

export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuTradingData((state: any) => {
        subscriber.next(state)
    })
})   

export const KUNGFU_GLOBAL_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuGlobalData((state: any) => {
        subscriber.next(state)
    })
})
       
export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger || {};
            const orders = Object.values(ledgerData.Order || {});
            const trades = Object.values(ledgerData.Trade || {});
            const positions = Object.values(ledgerData.Position || {});
            const assets = Object.values(ledgerData.Asset || {});
            const pnl = Object.values(ledgerData.AssetSnapshot || {});
            const orderStat = Object.values(ledgerData.OrderStat || {});
            const dailyAsset = Object.values(ledgerData.DailyAsset || {});

            return {
                orders: transformOrderTradeListToData(orders, type),
                trades: transformOrderTradeListToData(trades, type),
                positions: transformTradingItemListToData(positions, type),
                assets: transformAssetItemListToData(assets, type),
                pnl: transformTradingItemListToData(pnl, type),
                dailyPnl: transformTradingItemListToData(dailyAsset, type),
                orderStat: transformOrderStatListToData(orderStat)
            }
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return KUNGFU_GLOBAL_DATA_OBSERVER.pipe(
        map((data: any): any => {
            return {
                gatewayStates: dealGatewayStates(data.appStates || {})
            }
        })
    )
}




