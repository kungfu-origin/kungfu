import { KF_HOME, KF_CONFIG_PATH } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/busiUtils';
import { kungfu } from '__gUtils/kungfuUtils';
import { toDecimal } from '__gUtils/busiUtils';
import { readJsonSync } from '__gUtils/fileUtils';
import { offsetName, orderStatus, sideName, posDirection, priceType, hedgeFlag, instrumentType, volumeCondition, timeCondition } from "__gConfig/tradingConfig";
import { logger } from '../../utils/logUtils';



export const watcher: any = (() => {
    const kfSystemConfig: any = readJsonSync(KF_CONFIG_PATH)
    const bypassQuote = (kfSystemConfig.performance || {}).bypassQuote || false;

    if (process.env.APP_TYPE === 'cli') {
        const windowType = process.env.CLI_WINDOW_TYPE || '';
        const id = [process.env.APP_TYPE, windowType].join('');
        return kungfu.watcher(KF_HOME, kungfu.formatStringToHashHex(id), bypassQuote);
    }

    if (process.env.RENDERER_TYPE !== 'app') return {}

    const id = [process.env.APP_TYPE].join('');
    return kungfu.watcher(KF_HOME, kungfu.formatStringToHashHex(id), bypassQuote);
})()


export const startGetKungfuTradingData = (callback: Function, interval = 1000) => {
    if ((process.env.RENDERER_TYPE !== 'app') && (process.env.APP_TYPE !== 'cli')) return
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }

            if (watcher.isLive()) {
                watcher.step();
            }
            callback({
                ledger: watcher.ledger,
            });
            resolve();
        })
    }, interval);
}


export const startGetKungfuGlobalData = (callback: Function, interval = 1000) => {
    if ((process.env.RENDERER_TYPE !== 'app') && (process.env.APP_TYPE !== 'cli')) return
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }

            if (watcher.isLive()) {
                watcher.step();
            }

            callback({
                appStates: watcher.appStates,
            });
            resolve();
        })
    }, interval);
}


export const transformOrderTradeListToData = (list: any[], type: string) => {
    let data: StringToAnyObject = {};

    if (type === 'account') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.source);
            if (!location) return;
            const accountId = `${location.group}_${location.name}`;
            if (!accountId) return;
            if (!data[accountId]) data[accountId] = [];
            data[accountId].push(item)
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.dest);
            if (!location) return;
            const clientId = location.name;
            if (!clientId) return;
            if (!data[clientId]) data[clientId] = [];
            data[clientId].push(item)
        })
    }
    return data;
}


export const transformOrderStatListToData = (list: any[]) => {
    let data: StringToAnyObject = {};
    list.kfForEach((item: any) => {
        data[item.order_id.toString()] = dealOrderStat(item);
    })
    return data;
}


export const transformTradingItemListToData = (list: any[], type: string) => {
    let data: StringToAnyObject = {}
    if (type === 'account') {
        list.kfForEach((item: any) => {
            const accountId = `${item.source_id}_${item.account_id}`;
            if (!accountId) return;
            const ledgerCategory = +item.ledger_category;
            if (ledgerCategory === 0) {
                if (!data[accountId]) data[accountId] = [];
                data[accountId].push(item)
            }
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const clientId = item.client_id;
            if (!clientId) return;
            const ledgerCategory = +item.ledger_category;
            if (ledgerCategory === 1) {
                if (!data[clientId]) data[clientId] = [];
                data[clientId].push(item)
            }
        })
    }
    return data
}


export const transformAssetItemListToData = (list: any[], type: string) => {
    let accountIdClientIdData = transformTradingItemListToData(list, type);
    Object.keys(accountIdClientIdData || {}).forEach((id: string) => {
        const valueData = accountIdClientIdData[id]
            .reduce((a: any, b: any) => {
                return {
                    ...a,
                    ...b
                }
            })
        accountIdClientIdData[id] = dealAsset(valueData)
    })

    return accountIdClientIdData
}




export function decodeKungfuLocation(sourceOrDest: string): KungfuLocation {
    if (!sourceOrDest) return {
        category: 'td',
        group: 'node',
        name: '',
        mode: 'live'
    }

    const location: KungfuLocation = watcher.getLocation(sourceOrDest)
    return location
}


// ========================== 交易数据处理 start ===========================

function resolveClientId(dest: string): string {
    const kungfuLocation: KungfuLocation = decodeKungfuLocation(dest);
    if (!kungfuLocation) return '';
    if (kungfuLocation.group === 'node') return '手动'
    const name = kungfuLocation.name;
    return name
}


function resolveAccountId(source: string, dest: string): string {
    const kungfuLocationSource: KungfuLocation = decodeKungfuLocation(source)
    const kungfuLocationDest: KungfuLocation = decodeKungfuLocation(dest)
    
    const name = kungfuLocationSource.group + '_' + kungfuLocationSource.name;
    const group = kungfuLocationDest.group === 'node' ? '手动' : '';
    return [group, name].join(' ')
}


export const dealOrder = (item: OrderInputData): OrderData => {
    const updateTime = item.update_time || item.insert_time;
    return {
        id: [item.order_id.toString(), item.account_id.toString()].join('-'),
        updateTime: kungfu.formatTime(updateTime, '%H:%M:%S'),
        updateTimeMMDD: kungfu.formatTime(updateTime, '%m/%d %H:%M:%S'),
        updateTimeNum: +Number(updateTime || 0),

        orderId: item.order_id.toString(),
        parentId: item.parent_id.toString(),
        
        instrumentId: item.instrument_id,
        instrumentType: instrumentType[item.instrument_type],
        instrumentTypeOrigin: item.instrument_type,
        exchangeId: item.exchange_id,
        
        side: sideName[item.side] ? sideName[item.side] : '--',
        sideOrigin: item.side,
        offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
        offsetOrigin: item.offset,
        hedgeFlag: hedgeFlag[item.hedge_flag] ? hedgeFlag[item.hedge_flag] : '--',
        hedgeFlagOrigin: item.hedge_flag,

        priceType: priceType[item.price_type],
        priceTypeOrigin: item.price_type,
        volumeCondition: volumeCondition[item.volume_condition],
        timeCondition: timeCondition[item.time_condition],

        limitPrice: toDecimal(item.limit_price, 3) || '--',
        frozenPrice: toDecimal(item.frozen_price, 3) || '--',
        
        volume: item.volume.toString(),
        volumeTraded: item.volume_traded.toString() + "/" + item.volume.toString(),
        volumeLeft: item.volume_left.toString(),

        statusName: orderStatus[item.status],
        status: item.status,

        tax: item.tax,
        comission: item.commission,

        errorId: item.error_id,
        errorMsg: item.error_msg.toString(),

        clientId: resolveClientId(item.dest || ''),
        accountId: resolveAccountId(item.source, item.dest),
        sourceId: item.source_id,
       
        source: item.source,
        dest: item.dest
    }
}


export const dealTrade = (item: TradeInputData): TradeData => {
    const updateTime = item.trade_time || item.update_time;
    return {
        id: [item.account_id.toString(), item.trade_id.toString(), updateTime.toString()].join('_'),
        updateTime: kungfu.formatTime(updateTime, '%H:%M:%S'),
        updateTimeMMDD: kungfu.formatTime(updateTime, '%m/%d %H:%M:%S'),
        orderId: item.order_id.toString(),
        updateTimeNum: +Number(updateTime || 0),
        instrumentId: item.instrument_id,
        side: sideName[item.side] ? sideName[item.side] : '--',
        offset: offsetName[item.offset],
        price: toDecimal(+item.price, 3),
        volume: Number(item.volume),
        clientId: resolveClientId(item.dest || ''),
        accountId: resolveAccountId(item.source, item.dest),
        sourceId: item.source_id,
        source: item.source
    }
}


export const dealPos = (item: PosInputData): PosData => {
    //item.type :'0': 未知, '1': 股票, '2': 期货, '3': 债券
    const direction: string = posDirection[item.direction] || '--';
    return {
        id: item.instrument_id + direction,
        instrumentId: item.instrument_id,
        direction,
        directionOrigin: item.direction,
        yesterdayVolume: Number(item.yesterday_volume),
        todayVolume: Number(item.volume) - Number(item.yesterday_volume),
        totalVolume: Number(item.volume),
        avgPrice: toDecimal(item.avg_open_price || item.position_cost_price, 3) || '--',
        lastPrice: toDecimal(item.last_price, 3) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) + '' || '--',
        exchangeId: item.exchange_id
    }
}


export const dealAsset = (item: AssetInputData): AssetData => {
    return {
        accountId: `${item.source_id}_${item.account_id}`,
        clientId: item.client_id,
        initialEquity: toDecimal(item.initial_equity) || '--',
        staticEquity: toDecimal(item.static_equity) || '--',
        dynamicEquity: toDecimal(item.dynamic_equity) || '--',
        realizedPnl: toDecimal(item.realized_pnl) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) || '--',
        avail: toDecimal(item.avail) || '--',
        marketValue: toDecimal(item.market_value) || '--',
        margin: toDecimal(item.margin) || '--'
    }
}


export const dealOrderStat = (item: OrderStatInputData): OrderStatData => {
    const insertTime = item.insert_time;
    const ackTime = item.ack_time;
    const mdTime = item.md_time;
    const tradeTime = item.trade_time;

    const latencyTrade = +Number(Number(tradeTime - ackTime) / 1000).toFixed(0);
    const latencyNetwork = +Number(Number(ackTime - insertTime) / 1000).toFixed(0);
    const latencySystem = +toDecimal(Number(insertTime - mdTime) / 1000);

    return {
        ackTime: Number(ackTime),
        insertTime: Number(insertTime),
        mdTime: Number(mdTime),
        latencySystem: latencySystem > 0 ? latencySystem.toString() : '',
        latencyNetwork: latencyNetwork > 0 ? latencyNetwork.toString() : '',
        latencyTrade: latencyTrade > 0 ? latencyTrade.toString() : '',
        orderId: item.order_id.toString(),
        dest: item.dest,
        source: item.source
    }
}


export const dealGatewayStates = (gatewayStates: StringToStringObject): Array<MdTdState | {}> => {
    return Object.keys(gatewayStates)
        .map((key: string) => {
            const kungfuLocation = watcher.getLocation(key)
            if (!kungfuLocation) return {}
            switch (kungfuLocation.category) {
                case 'td':
                    return {
                        processId: `td_${kungfuLocation.group}_${kungfuLocation.name}`,
                        state: gatewayStates[key]
                    }
                case 'md':
                    return {
                        processId: `md_${kungfuLocation.group}`,
                        state: gatewayStates[key]
                    }
                default:
                    return {}
            }
        })
        .filter((mdTdState): MdTdState | {} => {
            return mdTdState !== {}
        })
}



// ========================== 交易数据处理 end ===========================

