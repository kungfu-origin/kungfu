import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    dealGatewayStates, 
    transformTradingItemListToData, 
    transformOrderTradeListToData,
    transformOrderInputListToData, 
    transformOrderStatListToData, 
    transformAssetItemListToData,

    dealOrderInput,
    dealOrder,
    dealTrade,
    dealPos,
    dealAsset,
    dealOrderStat,
    dealSnapshot,
    dealQuote
} from '__io/kungfu/watcher';

import { startGetKungfuTradingData, watcher } from '__io/kungfu/watcher';

export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuTradingData((state: any) => {
        subscriber.next(state)
    }, 1500)
})

function ensureLeaderData (data: any, key = '') {
    if (!key) {
        return data ? data.list() : []
    }

    return data ? data.sort(key) : []
}
 
export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            const ledgerData = data.ledger || {};
            const orderInputs = ensureLeaderData(ledgerData.OrderInput, 'insert_time').slice(0, 200).map((item: OrderInputOriginData) => dealOrderInput(item));
            const orders = ensureLeaderData(ledgerData.Order, 'update_time').slice(0, 200).map((item: OrderOriginData) => dealOrder(item));
            const trades = ensureLeaderData(ledgerData.Trade, 'trade_time').slice(0, 200).map((item: TradeOriginData) => dealTrade(item));
            const positions = ensureLeaderData(ledgerData.Position).map((item: PosOriginData) => dealPos(item));
            const assets = ensureLeaderData(ledgerData.Asset).map((item: AssetOriginData) => dealAsset(item));
            const orderStat = ensureLeaderData(ledgerData.OrderStat).map((item: OrderStatOriginData) => dealOrderStat(item));
            const pnl = ensureLeaderData(ledgerData.AssetSnapshot, 'update_time').map((item: AssetSnapshotOriginData) => dealSnapshot(item));
            const dailyAsset = ensureLeaderData(ledgerData.DailyAsset, 'trading_day').map((item: AssetSnapshotOriginData) => dealSnapshot(item));

            
            return {
                orders: transformOrderTradeListToData(orders, type),
                orderInputs: transformOrderInputListToData(orderInputs, type),
                ordersByTicker: orders,
                trades: transformOrderTradeListToData(trades, type),
                tradesByTicker: trades,
                positions: transformTradingItemListToData(positions, type),
                positionsByTicker: transformTradingItemListToData(positions, 'ticker'),
                assets: transformAssetItemListToData(assets, type),
                orderStat: transformOrderStatListToData(orderStat),
                pnl: transformTradingItemListToData(pnl, type),
                dailyPnl: transformTradingItemListToData(dailyAsset, type),
            }
        })
    )
}

export const buildInstrumentsPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            const ledgerData = data.ledger || {};
            const instruments = ensureLeaderData(ledgerData.Instrument);
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
            const orders = ensureLeaderData(ledgerData.Order, 'update_time').slice(0, 200).map((item: OrderOriginData) => dealOrder(item));
            const orderStat = ensureLeaderData(ledgerData.OrderStat).map((item: OrderStatOriginData) => dealOrderStat(item));
            
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
            const quotes = ensureLeaderData(ledgerData.Quote).map((item: QuoteOriginData) => dealQuote(item));
            return transformTradingItemListToData(quotes, 'quote')
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any): any => {
            return {
                watcherIsLive: watcher.isLive() || false,
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
            return {
                timeValueList: timeValueList.slice(0, 300)
            }
        })
    )
}


