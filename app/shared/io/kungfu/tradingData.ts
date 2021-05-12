import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    startGetKungfuTradingData,
    watcher,

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
} from '__io/kungfu/watcher';

import { ensureLeaderData, resolveInstruments } from '__gUtils/busiUtils';

export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    subscriber.next({})
    startGetKungfuTradingData((watcher: any) => {
        subscriber.next(watcher)
    }, 500)
})
 
export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map(() => {

            const ledgerData = watcher.ledger
            const orderInputs = ensureLeaderData(ledgerData.OrderInput, 'insert_time').slice(0, 100).map((item: OrderInputOriginData) => dealOrderInput(item));
            const orders = ensureLeaderData(ledgerData.Order, 'update_time').slice(0, 100).map((item: OrderOriginData) => dealOrder(item));
            const trades = ensureLeaderData(ledgerData.Trade, 'trade_time').slice(0, 100).map((item: TradeOriginData) => dealTrade(item));
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
        map(() => {
            const ledgerData = watcher.ledger
            const instruments = ensureLeaderData(ledgerData.Instrument);
            const instrumentsAfterFilter = instruments
                .filter((item: InstrumentOriginData) => {
                    //普通股票 期货 股票期权 基金 科创板股票 指数
                    if (item.instrument_type === 1) return true;
                    if (item.instrument_type === 2) return true;
                    if (item.instrument_type === 4) return true;
                    if (item.instrument_type === 5) return true;
                    if (item.instrument_type === 6) return true;
                    if (item.instrument_type === 7) return true;

                    return false
                })
            return {
                instruments: resolveInstruments(instrumentsAfterFilter)
            }
        })
    )

}

export const buildAllOrdersPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map(() => {
            const ledgerData = watcher.ledger
            const orders = ensureLeaderData(ledgerData.Order, 'update_time').slice(0, 100).map((item: OrderOriginData) => dealOrder(item));
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
        map(() => {
            const ledgerData = watcher.ledger
            const quotes = ensureLeaderData(ledgerData.Quote)
            return quotes
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map(() => {
            return {
                watcherIsLive: watcher.isLive() || false,
                gatewayStates: dealGatewayStates(watcher.appStates)
            }
        })
    )
}

export const buildTaskDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map(() => {
            const stateData = watcher.state;
            const timeValueList = ensureLeaderData(stateData.TimeValue.filter('tag_c', 'task'), 'update_time').slice(0, 100)
            return {
                timeValueList: timeValueList
            }
        })
    )
}


