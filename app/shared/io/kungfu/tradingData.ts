import { Observable, Subscriber } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
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

import { setTimerPromiseTask, ensureLeaderData, resolveInstruments } from '__gUtils/busiUtils';



export const KUNGFU_TRADING_DATA_OBSERVER = new Observable(subscriber => {
    
    if (watcher.noWathcer) return;

    if (process.env.APP_TYPE !== 'daemon') {
        if (process.env.APP_TYPE !== 'cli') {
            return;
        }
    }

    setTimerPromiseTask(() => {

        return new Promise(resolve => {

            const ledgerData = watcher.ledger
            const orderInputs = ensureLeaderData(ledgerData.OrderInput, 'insert_time').slice(0, 100).map((item: OrderInputOriginData) => dealOrderInput(item));
            const orders = ensureLeaderData(ledgerData.Order, 'update_time').slice(0, 100).map((item: OrderOriginData) => dealOrder(item));
            const trades = ensureLeaderData(ledgerData.Trade, 'trade_time').slice(0, 100).map((item: TradeOriginData) => dealTrade(item));
            const positions = ensureLeaderData(ledgerData.Position).map((item: PosOriginData) => dealPos(item));
            const positionsByTicker = transformTradingItemListToData(positions, 'ticker');
            const assets = ensureLeaderData(ledgerData.Asset).map((item: AssetOriginData) => dealAsset(item));
            const orderStat = ensureLeaderData(ledgerData.OrderStat).map((item: OrderStatOriginData) => dealOrderStat(item));
            const orderStatResolved = transformOrderStatListToData(orderStat);
            const pnl = ensureLeaderData(ledgerData.AssetSnapshot, 'update_time').map((item: AssetSnapshotOriginData) => dealSnapshot(item));
            const dailyAsset = ensureLeaderData(ledgerData.DailyAsset, 'trading_day').map((item: AssetSnapshotOriginData) => dealSnapshot(item));
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
                });
            const quotes = ensureLeaderData(ledgerData.Quote)
            
    
            const accountTradingDataPipeData = {
                orders: transformOrderTradeListToData(orders, 'account'),
                orderInputs: transformOrderInputListToData(orderInputs, 'account'),
                ordersByTicker: orders,
                trades: transformOrderTradeListToData(trades, 'account'),
                tradesByTicker: trades,
                positions: transformTradingItemListToData(positions, 'account'),
                positionsByTicker,
                assets: transformAssetItemListToData(assets, 'account'),
                orderStat: orderStatResolved,
                pnl: transformTradingItemListToData(pnl, 'account'),
                dailyPnl: transformTradingItemListToData(dailyAsset, 'account'),
            }
    
            const strategyTradingDataPipeData = {
                orders: transformOrderTradeListToData(orders, 'strategy'),
                orderInputs: transformOrderInputListToData(orderInputs, 'strategy'),
                ordersByTicker: orders,
                trades: transformOrderTradeListToData(trades, 'strategy'),
                tradesByTicker: trades,
                positions: transformTradingItemListToData(positions, 'strategy'),
                positionsByTicker,
                assets: transformAssetItemListToData(assets, 'strategy'),
                orderStat: orderStatResolved,
                pnl: transformTradingItemListToData(pnl, 'strategy'),
                dailyPnl: transformTradingItemListToData(dailyAsset, 'strategy'),
            }
            
            const instrumentsPipeData = resolveInstruments(instrumentsAfterFilter);
    
            const allOrdersPipeData = {
                orderStat: orderStatResolved,
                orders
            }
    
            subscriber.next({
                accountTradingDataPipeData,
                strategyTradingDataPipeData,
                instrumentsPipeData,
                allOrdersPipeData,
                quotes,
                globalPipeData: {
                    watcherIsLive: watcher.isLive() || false,
                    gatewayStates: dealGatewayStates(watcher.appStates)
                }
            })

            resolve(true)
        })

    }, 1000)

})

export const KUNGFU_TASK_DATA_OBSERVER = new Observable(subscriber => {
    if (watcher.noWathcer) return;

    if (process.env.APP_TYPE !== 'renderer') {
            return;
    }

    setTimerPromiseTask(() => {
        return new Promise(resolve => {
            const stateData = watcher.state;
            const timeValueList = ensureLeaderData(stateData.TimeValue.filter('tag_c', 'task'), 'update_time').slice(0, 100)
            subscriber.next({
                timeValueList: timeValueList
            })

            resolve(true)
        })
       
    }, 1000)
  
})

export const buildTradingDataPipe = (type: string) => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            return type === 'account' ? data.accountTradingDataPipeData : data.strategyTradingDataPipeData;
        })
    )
}

export const buildInstrumentsPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            return {
                instruments: data.instrumentsPipeData
            }
        })
    )

}

export const buildAllOrdersPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            return data.allOrdersPipeData
        })
    )
}

export const buildMarketDataPipe = () => {
    return KUNGFU_TRADING_DATA_OBSERVER.pipe(
        map((data: any) => {
            return data.quotes
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
    return KUNGFU_TASK_DATA_OBSERVER.pipe(
        map(() => {
            const stateData = watcher.state;
            const timeValueList = ensureLeaderData(stateData.TimeValue.filter('tag_c', 'task'), 'update_time').slice(0, 100)
            return {
                timeValueList: timeValueList
            }
        })
    )
}


