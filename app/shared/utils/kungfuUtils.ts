
import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask, toDecimal } from '__gUtils/BusiUtils';
import { offsetName, orderStatus, sideName, posDirection } from "__gConfig/tradingConfig";
import moment from 'moment';

// process.env.KF_LOG_LEVEL = 'trace';

export const kungfu = require('kungfu-core').kungfu;
export const longfist = kungfu.longfist;
export const kungfuConfigStore = kungfu.ConfigStore(KF_HOME)

export const watcher: any = (() => {
    if (process.env.RENDERER_TYPE !== 'app') return {}
    return kungfu.watcher(KF_HOME, ['watcher', process.env.APP_TYPE].join('_'));
})()

export const startGetKungfuTradingData = (callback: Function, interval = 1000) => {
    if (process.env.RENDERER_TYPE !== 'app') return 
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }
            
            if (watcher.isLive()) {
                watcher.step();
            }

            callback({ 
                ledger: Object.freeze(watcher.ledger), 
            });
            resolve();
        })
    }, interval);
}


export const startGetKungfuGlobalData = (callback: Function, interval = 1000) => {
    if (process.env.RENDERER_TYPE !== 'app') return 
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }
            
            if (watcher.isLive()) {
                watcher.step();
            }

            callback({ 
                tradingDay: watcher.tradingDay,
                appStates: watcher.appStates,
            });
            resolve();
        })
    }, interval);
}


export const getAllKfConfig = () => {
    return kungfuConfigStore.getAllConfig();
}


export const setKfConfig = (key: string, type: string, config: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.setConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode, config)
}


export const getKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.getConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)    
}


export const removeKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.removeConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)
}

export const getKfCommission = () => {
    return new Promise((resolve, reject) => {
        try {
            const commissionData = watcher.commission.getAllCommission();
            resolve(Object.values(commissionData || {}))
        } catch (err) {
            reject(err)
        }
    })
}

export const setKfCommission = (commissionItems: any) => {
    return new Promise((resolve, reject) => {
        const kfCommissionData = longfist.Commission()
        const comissionResolve = commissionItems.map((item: any) => {
            return {
                ...kfCommissionData,
                ...item
            }
        })

        const result = watcher.commission.setAllCommission(comissionResolve)

        if (result) {
            resolve(result)
        } else {
            reject(result)
        }
    })
    
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
        accountIdClientIdData[id] = valueData
    })
    return accountIdClientIdData
}


export function decodeKungfuLocation (sourceOrDest: string): KungfuLocation {
    if (!sourceOrDest) return {
        category: 'td',
        group: 'node',
        name: '',
        mode: 'live'
    }
    
    const location: KungfuLocation = watcher.getLocation(sourceOrDest)
    return location
}


export function encodeKungfuLocation (key: string, type: string): KungfuLocation {
    switch (type) {
        case 'td':
            const tdIdSplit = key.split('_');
            if (tdIdSplit.length !== 2) throw new Error(`td id ${key} is illegal!`);
            const sourceId = tdIdSplit[0];
            const accountId = tdIdSplit[1];
            ['td', sourceId, accountId, 'live']
            return {
                category: 'td',
                group: sourceId,
                name: accountId,
                mode: 'live' 
            } ;
        case 'md':
            if (!key) throw new Error(`md id ${key} is illegal!`);
            return {
                category: 'md',
                group: key,
                name: key,
                mode: 'live' 
            }
        case 'strategy':
            if (!key) throw new Error(`strategy id ${key} is illegal!`);
            return {
                category: 'strategy',
                group: 'default',
                name: key,
                mode: 'live' 
            }
        default:
            throw new Error(`unknow type ${type}`);
    }
}


// ========================== 交易数据处理 start ===========================

function resolveClientId (dest: string): string {
    const kungfuLocation: KungfuLocation = decodeKungfuLocation(dest)
    if (!kungfuLocation) return ''

    const group = kungfuLocation.group === 'node' ? '[手动]' : '';
    const name = kungfuLocation.name === 'watcher_renderer' ? '' : kungfuLocation.name

    return [ group, name ].join(' ')
}


function resolveAccountId (source: string, dest: string): string {
    const kungfuLocationSource: KungfuLocation = decodeKungfuLocation(source)
    const kungfuLocationDest: KungfuLocation = decodeKungfuLocation(dest)
    const name = kungfuLocationSource.group + '_' + kungfuLocationSource.name;
    const group = kungfuLocationDest.group === 'node' ? '[手动]' : '';
    return [ group, name ].join(' ')
}


export const dealOrder = (item: OrderInputData): OrderData => {
    const updateTime = +Number(item.update_time || item.insert_time || 0);
    return {
        id: [item.order_id.toString(), item.account_id.toString()].join('-'),
        updateTime: moment(+updateTime / 1000000).format("HH:mm:ss"),
        updateTimeNum: +updateTime,
        instrumentId: item.instrument_id,
        side: sideName[item.side] ? sideName[item.side] : '--',
        offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
        limitPrice: toDecimal(item.limit_price, 3) || '--',
        volumeTraded: item.volume_traded.toString() + "/" + item.volume.toString(),
        statusName: orderStatus[item.status],
        status: item.status,
        clientId: resolveClientId(item.dest || ''),
        accountId: resolveAccountId(item.source, item.dest),
        sourceId: item.source_id,
        orderId: item.order_id.toString(),
        exchangeId: item.exchange_id,
        source: item.source
    }
}


export const dealTrade = (item: TradeInputData): TradeData => {
    const updateTime = +Number(item.trade_time || item.update_time || 0);
    return {
        id: [item.account_id.toString(), item.trade_id.toString(), updateTime.toString()].join('_'),
        updateTime: moment(+updateTime / 1000000).format('HH:mm:ss'),
        updateTimeNum: +updateTime,
        instrumentId: item.instrument_id,
        side: sideName[item.side],
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
        yesterdayVolume: Number(item.yesterday_volume),
        todayVolume: Number(item.volume) - Number(item.yesterday_volume),
        totalVolume: Number(item.volume),
        avgPrice: toDecimal(item.avg_open_price || item.position_cost_price, 3) || '--',
        lastPrice: toDecimal(item.last_price, 3) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) + '' || '--'
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
    const insertTime = Number(item.insert_time);
    const ackTime = Number(item.ack_time);
    const mdTime = Number(item.md_time);

    return {
        ackTime,
        insertTime,
        mdTime,
        systemLatency: toDecimal((insertTime - mdTime) / 1000),
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

