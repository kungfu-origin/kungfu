import fse from 'fs-extra';
import { KF_RUNTIME_DIR, KF_CONFIG_PATH } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/busiUtils';
import { kungfu } from '__io/kungfu/kungfuUtils';
import { toDecimal, ensureNum } from '__gUtils/busiUtils';
import { OffsetName, OrderStatus, SideName, PosDirection, PriceType, HedgeFlag, InstrumentType, VolumeCondition, TimeCondition, allowShorted } from "kungfu-shared/config/tradingConfig";
import { logger } from '__gUtils/logUtils';


export const watcher: any = (() => {
    const kfSystemConfig: any = fse.readJsonSync(KF_CONFIG_PATH)
    const bypassQuote = (kfSystemConfig.performance || {}).bypassQuote || false;

    if (process.env.APP_TYPE === 'cli') {
        const windowType = process.env.CLI_WINDOW_TYPE || '';
        const id = [process.env.APP_TYPE, windowType].join('');
        return kungfu.watcher(KF_RUNTIME_DIR, kungfu.formatStringToHashHex(id), bypassQuote);
    }

    if (process.env.RENDERER_TYPE !== 'app') {
        if (process.env.RENDERER_TYPE !== 'makeOrder' && process.env.APP_TYPE != 'test') {
            return {}
        }
    }

    const id = [process.env.APP_TYPE, process.env.RENDERER_TYPE].join('');
    return kungfu.watcher(KF_RUNTIME_DIR, kungfu.formatStringToHashHex(id), bypassQuote);
})()


export const startGetKungfuTradingData = (callback: Function, interval = 1000) => {
    if (process.env.RENDERER_TYPE !== 'app') {
        if (process.env.RENDERER_TYPE !== 'makeOrder') {
            if (process.env.APP_TYPE !== 'cli') {
                return 
            }
        }
    }
    
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
                state: watcher.state,
                appStates: watcher.appStates,
            });
            resolve(true);
        })
    }, interval);
}


export const writeKungfu = (id: string, label: string, type: string, val: string) => {
    let data = kungfu.longfist.TimeValue();
    data.tag_a = id || '';
    data.tag_b = label || '';
    data.tag_c = type || '';
    data.value = val;
    data.update_time = watcher.now();
    watcher.publishState(data);
}



export const transformOrderTradeListToData = (list: any[], type: string) => {
    let data: StringToAnyObject = {};

    if (type === 'account') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.source);
            if (!location || !location.name) return;
            const accountId = `${location.group}_${location.name}`;
            if (!data[accountId]) data[accountId] = [];
            data[accountId].push(item)
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.dest);
            if (!location || !location.name) return;
            const clientId = location.name;
            if (!data[clientId]) data[clientId] = [];
            data[clientId].push(item)
        })
    }
    return data;
}

// source 跟 dest 跟普通相反，所以单独列出来
export const transformOrderInputListToData = (list: any[], type: string) => {
    let data: StringToAnyObject = {};

    if (type === 'account') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.dest);
            if (!location || !location.name) return;
            const accountId = `${location.group}_${location.name}`;
            if (!data[accountId]) data[accountId] = [];
            data[accountId].push(item)
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.source);
            if (!location || !location.name) return;
            const clientId = location.name;
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
            if (!item.account_id) return;
            const accountId = `${item.source_id}_${item.account_id}`;
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
    } else if (type === 'ticker') {
        list.kfForEach((item: any) => {
            if (!item.account_id) return;
            if (!item.instrument_id) return;
            const instrumentId = `${item.instrument_id}_${item.direction}`;
            if (!data[instrumentId]) data[instrumentId] = [];
            data[instrumentId].push(item)
        })

    } else if (type === 'quote') {
        list.kfForEach((item: any) => {
            const instrumentId = `${item.exchange_id}_${item.instrument_id}_${item.source_id}`;
            data[instrumentId] = item;
        })
    }

    return data
}

export const transformPositionByTickerByMerge = (positionsByTicker: { [propname: string]: PosOriginData[] }, type: string) => {
    const positionsByTickerList = Object.values(positionsByTicker)
        .map((tickerList: PosOriginData[]) => {
            const tickerListResolved = tickerList
            .filter(item => {
                if (!item.account_id) return false;
                if (type === 'account') return !item.client_id;
                if (type === 'strategy') return item.client_id;
                return true;
            })

            if (!tickerListResolved.length) return null;
            return tickerListResolved.reduce((item1: PosOriginData, item2: PosOriginData) => {
                return {
                    ...item1,
                    yesterday_volume: item1.yesterday_volume + item2.yesterday_volume,
                    volume: item1.volume + item2.volume,
                    unrealized_pnl: item1.unrealized_pnl + item2.unrealized_pnl
                }
            })
        })
        .filter(item => !!item)
    
    return positionsByTickerList
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


interface SourceDest {
    sourceGroup:string;
    sourceName:string;
    destGroup: string;
}

function resolveSourceDest (source: string, dest: string): SourceDest {
    const kungfuLocationSource: KungfuLocation = decodeKungfuLocation(source)
    const kungfuLocationDest: KungfuLocation = decodeKungfuLocation(dest)
    
    return {
        sourceGroup: kungfuLocationSource.group.toString(),
        sourceName: kungfuLocationSource.name.toString(),
        destGroup: kungfuLocationDest.group.toString()
    }
}

function resolveAccountId(source: string, dest: string, parent_id: bigint): string {
    const { sourceName, sourceGroup, destGroup  } = resolveSourceDest(source, dest)
    const name = sourceGroup + '_' + sourceName;
    let mark = ''
    if (destGroup === 'node') {
        if (+parent_id.toString()) {
            mark = '任务'
        } else {
            mark = '手动'
        }
    } else {
        if (+parent_id.toString()) {
            mark = '手动'
        }
    }
    return [name, mark].join(' ')
}

function resolveClientId(dest: string, parent_id: bigint): string {
    const kungfuLocation: KungfuLocation = decodeKungfuLocation(dest);
    if (!kungfuLocation) return '';
    if (kungfuLocation.group === 'node') {
        if (+parent_id.toString()) {
            return '任务'
        } else {
            return '手动'
        }
    } else {
        if (+parent_id.toString()) {
            return `${kungfuLocation.name} 手动`
        } else {
            return kungfuLocation.name
        }
    }
}

export const dealOrderInput = (item: OrderInputOriginData): OrderInputData => {
    const { source, dest, instrument_type, ts, side, offset, hedge_flag, price_type } = item;
    //与正常相反 dest source
    const sourceId =  resolveSourceDest(dest, source).sourceGroup;
    const accountId = resolveAccountId(dest, source, item.parent_id);

    return {
        id: item.order_id.toString(),
        orderId: item.order_id.toString(),
        parentId: item.parent_id.toString(),
        updateTime: kungfu.formatTime(ts, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(ts, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(ts || 0),

        instrumentId: item.instrument_id,
        exchangeId: item.exchange_id,
        sourceId: sourceId,
        accountId: accountId,
        instrumentType: InstrumentType[instrument_type],
        instrumentTypeOrigin: instrument_type,
        limitPrice: toDecimal(item.limit_price, 3) || '--',
        frozenPrice: toDecimal(item.frozen_price, 3) || '--',
        volume: item.volume.toString(),

        side: SideName[side] ? SideName[side] : '--',
        sideOrigin: side,
        offset: OffsetName[offset],
        offsetOrigin: offset,
        hedgeFlag: HedgeFlag[hedge_flag] ? HedgeFlag[hedge_flag] : '--',
        hedgeFlagOrigin: hedge_flag,

        priceType: PriceType[price_type],
        priceTypeOrigin: price_type,

        source,
        dest
        
    }
}


export const dealOrder = (item: OrderOriginData): OrderData => {
    const { source, dest, instrument_type, update_time, side, offset, hedge_flag, price_type } = item;
    const sourceId =  resolveSourceDest(source, dest).sourceGroup;
    const errMsg = item.error_msg || OrderStatus[item.status];
    const accountId = resolveAccountId(source, dest, item.parent_id);
  
    return {
        id: item.order_id.toString(),
        updateTime: kungfu.formatTime(update_time, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(update_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(update_time || 0),

        orderId: item.order_id.toString(),
        parentId: item.parent_id.toString(),
        
        instrumentId: item.instrument_id,
        instrumentType: InstrumentType[instrument_type],
        instrumentTypeOrigin: instrument_type,
        exchangeId: item.exchange_id,
        
        side: SideName[side] ? SideName[side] : '--',
        sideOrigin: side,
        offset: OffsetName[offset],
        offsetOrigin: offset,
        hedgeFlag: HedgeFlag[hedge_flag] ? HedgeFlag[hedge_flag] : '--',
        hedgeFlagOrigin: hedge_flag,

        priceType: PriceType[price_type],
        priceTypeOrigin: price_type,
        volumeCondition: VolumeCondition[item.volume_condition],
        timeCondition: TimeCondition[item.time_condition],

        limitPrice: toDecimal(item.limit_price, 3) || '--',
        frozenPrice: toDecimal(item.frozen_price, 3) || '--',
        
        volume: item.volume.toString(),
        volumeTraded: item.volume_traded.toString() + "/" + item.volume.toString(),
        volumeLeft: item.volume_left.toString(),

        statusName: +item.status !== 4 ? OrderStatus[item.status] : errMsg,
        status: item.status,

        tax: item.tax,
        comission: item.commission,

        errorId: item.error_id,
        errorMsg: errMsg,

        clientId: resolveClientId(dest || '', item.parent_id),
        accountId: accountId,
        sourceId: sourceId,
       
        source: source.toString(),
        dest: dest.toString()
    }
}


export const dealTrade = (item: TradeOriginData): TradeData => {
    const { source, dest, instrument_type, trade_time, side, offset, hedge_flag, parent_order_id } = item;
    //这个update会用延迟统计里的，因为目前是交易所时间，需要本地时间
    const sourceId =  resolveSourceDest(source, dest).sourceGroup;
    const accountId = resolveAccountId(source, dest, parent_order_id);

    return {
        id: [item.account_id.toString(), item.trade_id.toString(), trade_time.toString()].join('_'),
        updateTime: kungfu.formatTime(trade_time, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(trade_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(trade_time || 0),
        orderId: item.order_id.toString(),
        parentOrderId: parent_order_id.toString(),

        instrumentId: item.instrument_id,
        instrumentType: InstrumentType[instrument_type],
        instrumentTypeOrigin: instrument_type,
        exchangeId: item.exchange_id,

        side: SideName[side] ? SideName[side] : '--',
        sideOrigin: side,
        offset: OffsetName[offset],
        offsetOrigin: offset,
        hedgeFlag: HedgeFlag[hedge_flag] ? HedgeFlag[hedge_flag] : '--',
        hedgeFlagOrigin: hedge_flag,

        price: toDecimal(+item.price, 3) || '--',
        volume: Number(item.volume),

        clientId: resolveClientId(dest || '', parent_order_id),
        accountId: accountId,
        sourceId: sourceId,
    
        source: source.toString(),
        dest: dest.toString(),

        tax: toDecimal(+item.tax, 3) || '--',
        commission: toDecimal(+item.commission, 3) || '--'
    }
}


export const dealPos = (item: PosOriginData): PosData => {
    //item.type :'0': 未知, '1': 股票, '2': 期货, '3': 债券
    const direction: string = PosDirection[item.direction] || '--';
    const avgPrice: string = toDecimal(item.avg_open_price || item.position_cost_price, 3) || ''
    return {
        id: item.instrument_id + direction,
        instrumentId: item.instrument_id,
        instrumentType: item.instrument_type,
        direction,
        directionOrigin: item.direction,
        yesterdayVolume: Number(item.yesterday_volume),
        todayVolume: Number(item.volume) - Number(item.yesterday_volume),
        totalVolume: Number(item.volume),
        avgPrice: avgPrice || '--',
        lastPrice: toDecimal(item.last_price, 3) || '--',
        totalPrice: toDecimal(+avgPrice * Number(item.volume), 3) || '--',
        totalMarketPrice: toDecimal(item.last_price * Number(item.volume), 3) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) + '' || '--',
        exchangeId: item.exchange_id,
        accountId: item.account_id,
        sourceId: item.source_id,
        clientId: item.client_id,
        accountIdResolved: `${item.source_id}_${item.account_id}`
    }
}


export const dealAsset = (item: AssetOriginData): AssetData => {
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


export const dealOrderStat = (item: OrderStatOriginData): OrderStatData => {
    const { insert_time, ack_time, md_time, trade_time } = item;
    const latencyTrade = +toDecimal(Number(trade_time - ack_time) / 1000);
    const latencyNetwork = +toDecimal(Number(ack_time - insert_time) / 1000);
    const latencySystem = +toDecimal(Number(insert_time - md_time) / 1000);

    return {
        ackTime: Number(ack_time),
        insertTime: Number(insert_time),
        mdTime: Number(md_time),
        latencySystem: latencySystem > 0 ? latencySystem.toString() : '',
        latencyNetwork: latencyNetwork > 0 ? latencyNetwork.toString() : '',
        latencyTrade: latencyTrade > 0 ? latencyTrade.toString() : '',
        tradeTime: kungfu.formatTime(trade_time, '%H:%M:%S.%N').slice(0, 12),
        tradeTimeMMDD: kungfu.formatTime(trade_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        tradeTimeNum: +Number(trade_time || 0),

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


export const dealQuote = (quote: QuoteDataInput): QuoteData => {
    return {
        id: quote.exchange_id + quote.source_id + quote.instrument_id.toString() + String(quote.data_time),
        closePrice: toDecimal(ensureNum(quote.close_price), 3),
        dataTime: kungfu.formatTime(quote.data_time, '%Y-%m-%d %H:%M:%S'),
        dataTimeNumber: quote.data_time.toString(),
        exchangeId: quote.exchange_id,
        highPrice: toDecimal(ensureNum(quote.high_price), 3),
        instrumentId: quote.instrument_id,
        instrumentType: InstrumentType[quote.instrument_type],
        instrumentTypeOrigin: quote.instrument_type,
        lastPrice: toDecimal(ensureNum(quote.last_price), 3),
        lowPrice: toDecimal(ensureNum(quote.low_price), 3),
        lowerLimitPrice: toDecimal(ensureNum(quote.lower_limit_price), 3),
        openInterest: ensureNum(quote.open_interest),
        openPrice: toDecimal(ensureNum(quote.open_price), 3),
        preClosePrice: toDecimal(ensureNum(quote.pre_close_price), 3),
        preOpenInterest: ensureNum(quote.pre_open_interest),
        preSettlementPrice: toDecimal(ensureNum(quote.pre_settlement_price), 3),
        settlementPrice: toDecimal(ensureNum(quote.settlement_price), 3),
        sourceId: quote.source_id,
        tradingDay: quote.trading_day,
        turnover: ensureNum(quote.turnover),
        upperLimitPrice: toDecimal(ensureNum(quote.upper_limit_price), 3),
        volume: ensureNum(quote.volume),
        askPrices: quote.ask_price.map(num => toDecimal(ensureNum(num), 3)) || [],
        askVolumes: quote.ask_volume.map((num: BigInt) => num.toString()) || [],
        bidPrices: quote.bid_price.map(num => toDecimal(ensureNum(num), 3)) || [],
        bidVolumes: quote.bid_volume.map((num: BigInt) => num.toString()) || [],
    }
       

}



// ========================== 交易数据处理 end ===========================

