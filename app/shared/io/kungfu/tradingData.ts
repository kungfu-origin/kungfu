import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    dealGatewayStates, 
    transformTradingItemListToData, 
    transformOrderTradeListToData,
    transformOrderInputListToData, 
    transformOrderStatListToData, 
    transformAssetItemListToData 
} from '__io/kungfu/watcher';

import { startGetKungfuTradingData } from '__io/kungfu/watcher';

export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuTradingData((state: any) => {
        subscriber.next(state)
    })
})
       
export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger || {};
            const orders = ledgerData.Order ? ledgerData.Order.sort('ts') : [];
            const orderInputs = ledgerData.OrderInput ? ledgerData.OrderInput.sort('ts') : [];
            const trades = ledgerData.Trade ? ledgerData.Trade.sort('ts') : [];
            const positions = Object.values(ledgerData.Position || {});
            const assets = Object.values(ledgerData.Asset || {});
            const pnl = Object.values(ledgerData.AssetSnapshot || {});
            const orderStat = Object.values(ledgerData.OrderStat || {});
            const dailyAsset = Object.values(ledgerData.DailyAsset || {});
            const instruments = Object.values(ledgerData.Instrument || {});
            
            return {
                orders: transformOrderTradeListToData(orders, type),
                orderInputs: transformOrderInputListToData(orderInputs, type),
                ordersByTicker: orders,
                trades: transformOrderTradeListToData(trades, type),
                tradesByTicker: trades,
                positions: transformTradingItemListToData(positions, type),
                positionsByTicker: transformTradingItemListToData(positions, 'ticker'),
                assets: transformAssetItemListToData(assets, type),
                pnl: transformTradingItemListToData(pnl, type),
                dailyPnl: transformTradingItemListToData(dailyAsset, type),
                orderStat: transformOrderStatListToData(orderStat),
                instruments: instruments
            }
        })
    )
}

export const buildInstrumentsPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            const ledgerData = data.ledger || {};
            const instruments = Object.values(ledgerData.Instrument || {});
            return {
                instruments
            }
        })
    )

}

export const buildAllOrdersPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger || {};
            const orders = ledgerData.Order ? ledgerData.Order.sort('update_time') : [];
            const orderStat = Object.values(ledgerData.OrderStat || {});
            
            return {
                orderStat: transformOrderStatListToData(orderStat),
                orders
            }
        })
    )
}

export const buildMarketDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger || {};
            const quotes = Object.values(ledgerData.Quote || {});
            return transformTradingItemListToData(quotes, 'quote')
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            return {
                gatewayStates: dealGatewayStates(data.appStates || {})
            }
        })
    )
}

export const buildTaskDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const stateData = data.state || {};
            const timeValueList = stateData.TimeValue ? stateData.TimeValue.filter('tag_c', 'task').sort('update_time') : [];
            return timeValueList.slice(0, 300)
        })
    )
}


