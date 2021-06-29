import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';

import { 
    watcher,

    dealGatewayStates, 
    transformTradingItemListToData, 
    transformAssetItemListToData,
    dealPos,
    dealAsset,
    dealSnapshot,
} from '__io/kungfu/watcher';

import { setTimerPromiseTask, ensureLedgerData } from '__gUtils/busiUtils';


const deamonDataSubject: any = new Subject();

const appDataSubject: any = new Subject();



(() => {

    if (watcher.noWatcher) {
        return;
    };
    
    console.log(process.env.APP_TYPE, '===')
    
    if (process.env.APP_TYPE !== 'daemon') {
        return
    };

    setTimerPromiseTask(async () => {
            const ledgerData = watcher.ledger;
            const positions = ensureLedgerData(ledgerData.Position).map((item: PosOriginData) => dealPos(item));
            const positionsByTicker = transformTradingItemListToData(positions, 'ticker');
            const assets = ensureLedgerData(ledgerData.Asset).map((item: AssetOriginData) => dealAsset(item));
            const pnl = ensureLedgerData(ledgerData.AssetSnapshot, 'update_time').map((item: AssetSnapshotOriginData) => dealSnapshot(item));
            const dailyAsset = ensureLedgerData(ledgerData.DailyAsset, 'trading_day').map((item: AssetSnapshotOriginData) => dealSnapshot(item));
            const quotes = ensureLedgerData(ledgerData.Quote);

            const accountTradingDataPipeData = {
                positions: transformTradingItemListToData(positions, 'account'),
                positionsByTicker,
                assets: transformAssetItemListToData(assets, 'account'),
                pnl: transformTradingItemListToData(pnl, 'account'),
                dailyPnl: transformTradingItemListToData(dailyAsset, 'account'),
            }
    
            const strategyTradingDataPipeData = {
                positions: transformTradingItemListToData(positions, 'strategy'),
                assets: transformAssetItemListToData(assets, 'strategy'),
                pnl: transformTradingItemListToData(pnl, 'strategy'),
                dailyPnl: transformTradingItemListToData(dailyAsset, 'strategy'),
            }
    
            deamonDataSubject.next({
                accountTradingDataPipeData,
                strategyTradingDataPipeData,
                quotes,
                globalPipeData: {
                    daemonIsLive: watcher.isLive(),
                }
            })

            return true

    }, 1000)

})();

(() => {

    if (watcher.noWatcher) {
        return;
    }

    if (process.env.RENDERER_TYPE !== 'app') {
        if (process.env.APP_TYPE !== 'cli') {
            return;
        };
    }

    setTimerPromiseTask(() => {
        return new Promise(resolve => {
            const ledgerData = watcher.ledger;

            let instrumentsAfterFilter: InstrumentOriginData[] = []; 
            if (process.env.APP_TYPE !== 'cli') {
                const instruments = ensureLedgerData(ledgerData.Instrument);
                instrumentsAfterFilter = instruments
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

            }

            appDataSubject.next({
                instruments: instrumentsAfterFilter,
                gatewayStates: dealGatewayStates(watcher.appStates)
            })

            resolve(true)
        })
       
    }, 1000)

})();


export const buildTradingDataPipe = (type: string) => {
    return deamonDataSubject.pipe(
        map((data: any) => {
            return type === 'account' ? data.accountTradingDataPipeData : data.strategyTradingDataPipeData;
        })
    )
}

export const buildMarketDataPipe = () => {
    return deamonDataSubject.pipe(
        map((data: any) => {
            return data.quotes
        })
    )
}

export const buildKungfuGlobalDataPipe = () => {
    return deamonDataSubject.pipe(
        map((data: any) => {
            return data.globalPipeData
        })
    )
}

export const buildKungfuDataByAppPipe = () => {
    return appDataSubject
}

export const buildGatewayStatePipe = () => {
    return appDataSubject.pipe(
        map((data: any) => {
            return {
                gatewayStates: data.gatewayStates
            }
        })
    )
}

export const buildInstrumentsDataPipe = () => {
    return appDataSubject.pipe(
        map((data: any) => {
            return {
                instruments: data.instruments
            }
        })
    )
}

