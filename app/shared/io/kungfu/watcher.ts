import fse from 'fs-extra';
import { KF_RUNTIME_DIR, KF_CONFIG_PATH } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/busiUtils';
import { kungfu } from '__io/kungfu/kungfuUtils';
import { toDecimal, ensureNum, ensureLeaderData, addTwoItemByKeyForReduce, avgTwoItemByKeyForReduce } from '__gUtils/busiUtils';
import { OffsetName, OrderStatus, SideName, PosDirection, PriceType, HedgeFlag, InstrumentType, VolumeCondition, TimeCondition, allowShorted } from "kungfu-shared/config/tradingConfig";
import { logger } from '__gUtils/logUtils';


export const watcher: any = (() => {
    const kfSystemConfig: any = fse.readJsonSync(KF_CONFIG_PATH)
    const bypassQuote = (kfSystemConfig.performance || {}).bypassQuote || false;

    if (process.env.RENDERER_TYPE !== 'app') {
        if (process.env.APP_TYPE !== 'cli') {
            if (process.env.APP_TYPE !== 'daemon') {
                return {
                    noWatcher: true
                }
            }
        }
    }

    if (process.env.APP_TYPE === 'cli') {
        const windowType = process.env.CLI_WINDOW_TYPE || '';
        const id = [process.env.APP_TYPE, windowType].join('');
        return kungfu.watcher(KF_RUNTIME_DIR, kungfu.formatStringToHashHex(id), bypassQuote);
    }

    if (process.env.APP_TYPE === "daemon") {
        return kungfu.watcher(KF_RUNTIME_DIR, kungfu.formatStringToHashHex('kungfu_daemon'), false);
    }


    const id = [process.env.APP_TYPE, process.env.RENDERER_TYPE].join('');
    return kungfu.watcher(KF_RUNTIME_DIR, kungfu.formatStringToHashHex(id), true);
})()


export const startGetKungfuWathcerStep = (interval = 500) => {
    if (watcher.noWatcher) return;
    
    return setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }

            if (watcher.isLive()) {
                watcher.step();
            }
            
            resolve(true);
        })
    }, interval);
}


export const writeKungfuTimeValue = (id: string, label: string, type: string, val: string) => {
    let data = kungfu.longfist.TimeValue();
    data.tag_a = id || '';
    data.tag_b = label || '';
    data.tag_c = type || '';
    data.value = val;
    data.update_time = watcher.now();
    watcher.publishState(data);
}

function setImmediateIter (task: Function, count: number, cb: Function) {

    if (count >= 0) {
        setImmediate(() => {
            task(count)
            setImmediateIter(task, count--, cb)
        })
    } else {
        cb(count)
    } 
}

export const transformOrderTradeListToData = (list: OrderOriginData[] | TradeOriginData [], dealFunc: Function) => {
    let accountData: { [prop: string]: OrderData[] | TradeData[] } = {};
    let strategyData: { [prop: string]: OrderData[] | TradeData[] } = {};
    let tmpSourceIdData: { [prop: number]: KungfuLocation } = {};
    let tmpDestIdData: { [prop: number]: KungfuLocation } = {};

    return list.kfForEachAsync((item: OrderOriginData | TradeOriginData) => {
        const source = +item.source;
        const tmpAccountLocation = tmpSourceIdData[source];
        const accountLocation = tmpAccountLocation || decodeKungfuLocation(source);
        if (accountLocation && accountLocation.name) {
            if (!tmpAccountLocation) {
                tmpSourceIdData[source] = accountLocation;
            }

            const accountId = accountLocation.group + '_' + accountLocation.name;
            if (!accountData[accountId]) accountData[accountId] = [];
            if (accountData[accountId].length < 100) {
                accountData[accountId].push(dealFunc(item))
            }
        }

        const dest = +item.dest;
        const tmpDestLocation = tmpDestIdData[dest];
        const strategyLocation: KungfuLocation = tmpDestLocation || decodeKungfuLocation(dest);
        if (strategyLocation && strategyLocation.name) {
            if (!tmpDestLocation) {
                tmpDestIdData[dest] = strategyLocation;
            }

            const strategyId = strategyLocation.name;
            if (!strategyData[strategyId]) strategyData[strategyId] = [];
            if (strategyData[strategyId].length < 100) {
                strategyData[strategyId].push(dealFunc(item))
            }
        }
    })
    .then(() => {
        return {
            account: accountData,
            strategy: strategyData
        };
    })
}

// source 跟 dest 跟普通相反，所以单独列出来
export const transformOrderInputListToData = (list: OrderInputOriginData[], dealFunc: Function) => {
    let accountData: { [prop: string]: OrderInputData[] } = {};
    let strategyData: { [prop: string]: OrderInputData[] } = {};
    let tmpSourceIdData: { [prop: number]: KungfuLocation } = {};
    let tmpDestIdData: { [prop: number]: KungfuLocation } = {};

    return list.kfForEachAsync((item: OrderOriginData | TradeOriginData) => {
        const dest = +item.dest;
        const tmpAccountLocation = tmpDestIdData[dest];
        const accountLocation = tmpAccountLocation || decodeKungfuLocation(dest);
        if (accountLocation && accountLocation.name) {
            if (!tmpAccountLocation) {
                tmpDestIdData[dest] = accountLocation;
            }

            const accountId = accountLocation.group + '_' + accountLocation.name;
            if (!accountData[accountId]) accountData[accountId] = [];
            if (accountData[accountId].length < 100) {
                accountData[accountId].push(dealFunc(item))
            }
        }
        
        const source = +item.source;
        const tmpStrategyLocation = tmpSourceIdData[source];
        const strategyLocation: KungfuLocation = tmpStrategyLocation || decodeKungfuLocation(source);
        if (strategyLocation && strategyLocation.name) {
            if (!tmpStrategyLocation) {
                tmpSourceIdData[source] = strategyLocation;
            }

            const strategyId = strategyLocation.name;
            if (!strategyData[strategyId]) strategyData[strategyId] = [];
            if (strategyData[strategyId].length < 100) {
                strategyData[strategyId].push(dealFunc(item))
            }
        }
    })
    .then(() => {
        return {
            account: accountData,
            strategy: strategyData
        };
    })
}

export const transformOrderStatListToData = (list: OrderStatOriginData[]) => {
    let data: StringToAnyObject = {};
    list.kfForEach((item: OrderStatOriginData) => {
        data[item.order_id.toString()] = item;
    })
    return data;
}


export const transformTradingItemListToData = (list: any[], type: string) => {
    let data: StringToAnyObject = {}
    if (type === 'account') {
        list.kfForEach((item: any) => {
            if (!item.accountId) return;
            const accountId = `${item.sourceId}_${item.accountId}`;
            const ledgerCategory = +item.ledgerCategory;
            if (ledgerCategory === 0) {
                if (!data[accountId]) data[accountId] = [];
                data[accountId].push(item)
            }
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const clientId = item.clientId;
            if (!clientId) return;
            const ledgerCategory = +item.ledgerCategory;
            if (ledgerCategory === 1) {
                if (!data[clientId]) data[clientId] = [];
                data[clientId].push(item)
            }
        })
    } else if (type === 'ticker') {
        list.kfForEach((item: any) => {
            if (!item.accountId) return;
            if (!item.instrumentId) return;
            const instrumentId = `${item.instrumentId}_${item.directionOrigin}`;
            if (!instrumentId) return;
            if (!data[instrumentId]) data[instrumentId] = [];
            data[instrumentId].push(item)
        })

    } else if (type === 'quote') {
        list.kfForEach((item: any) => {
            const instrumentId = `${item.exchangeId}_${item.instrumentId}_${item.sourceId}`;
            if (!instrumentId) return;
            data[instrumentId] = item;
        })
    }

    return data
}

export const transformPositionByTickerByMerge = (positionsByTicker: { [propname: string]: PosData[] }, type: string) => {

    const positionsByTickerList = Object.keys(positionsByTicker)
        .map((key: string) => {
            const tickerList: PosData[] = positionsByTicker[key];
            const tickerListResolved = tickerList
            .filter(item => {
                if (!item.accountId) return false;
                if (type === 'account') return !item.clientId;
                if (type === 'strategy') return item.clientId;
                if (+item.totalVolume === 0) return false;
                return true;
            })

            if (!tickerListResolved.length) return null;    

            return tickerListResolved.reduce((item1: PosData, item2: PosData) => {
                return {
                    ...item1,

                    yesterdayVolume: addTwoItemByKeyForReduce(item1, item2, 'yesterdayVolume'),
                    todayVolume: addTwoItemByKeyForReduce(item1, item2, 'todayVolume'),
                    totalVolume: addTwoItemByKeyForReduce(item1, item2, 'totalVolume'),

                    avgPrice: avgTwoItemByKeyForReduce(item1, item2, 'avgPrice'),
                    totalPrice: addTwoItemByKeyForReduce(item1, item2, 'totalPrice'),
                    totalMarketPrice: addTwoItemByKeyForReduce(item1, item2, 'totalMarketPrice'),
                    unRealizedPnl: addTwoItemByKeyForReduce(item1, item2, 'unRealizedPnl'),
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
        accountIdClientIdData[id] = valueData
    })

    return accountIdClientIdData
}


export function decodeKungfuLocation(sourceOrDest: number): KungfuLocation {
    if (!sourceOrDest) return {
        category: 'td',
        group: 'node',
        name: '',
        mode: 'live'
    }

    const location: KungfuLocation = watcher.getLocation(sourceOrDest)
    return location
}

export function getTargetOrdersByParentId (Orders: any, parentId: string) {
    return ensureLeaderData(Orders.filter('parent_id', BigInt(parentId))).map((item: OrderOriginData) => dealOrder(item))
}

// ========================== 交易数据处理 start ===========================


interface SourceDest {
    sourceGroup:string;
    sourceName:string;
    destGroup: string;
}

function resolveSourceDest (source: number, dest: number): SourceDest {
    const kungfuLocationSource: KungfuLocation = decodeKungfuLocation(+source)
    const kungfuLocationDest: KungfuLocation = decodeKungfuLocation(+dest)
    
    return {
        sourceGroup: kungfuLocationSource.group.toString(),
        sourceName: kungfuLocationSource.name.toString(),
        destGroup: kungfuLocationDest.group.toString()
    }
}

function resolveAccountId(source: number, dest: number, parent_id: bigint): string {
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

function resolveClientId(dest: number, parent_id: bigint): string {
    const kungfuLocation: KungfuLocation = decodeKungfuLocation(+dest);
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
    const { source, dest, instrument_type, insert_time, side, offset, hedge_flag, price_type } = item;
    //与正常相反 dest source
    const sourceId =  resolveSourceDest(+dest, +source).sourceGroup;
    const accountId = resolveAccountId(+dest, +source, item.parent_id);

    return {
        id: item.order_id.toString(),
        orderId: item.order_id.toString(),
        parentId: item.parent_id.toString(),
        updateTime: kungfu.formatTime(insert_time, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(insert_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(insert_time || 0),

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
    const { source, dest, instrument_type, update_time, insert_time, side, offset, hedge_flag, price_type } = item;
    const sourceId =  resolveSourceDest(+source, +dest).sourceGroup;
    const accountId = resolveAccountId(+source, +dest, item.parent_id);
    const errMsg = item.error_msg || OrderStatus[item.status];
  
    return {
        id: item.order_id.toString(),
        updateTime: kungfu.formatTime(update_time || insert_time, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(update_time || insert_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(update_time || insert_time || 0),

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

        clientId: resolveClientId(+dest, item.parent_id),
        accountId: accountId,
        sourceId: sourceId,
       
        source: source,
        dest: dest
    }
}


export const dealTrade = (item: TradeOriginData): TradeData => {
    const { source, dest, instrument_type, trade_time, side, offset, hedge_flag, parent_order_id } = item;
    //这个update会用延迟统计里的，因为目前是交易所时间，需要本地时间
    const sourceId =  resolveSourceDest(+source, +dest).sourceGroup;
    const accountId = resolveAccountId(+source, +dest, parent_order_id);

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

        clientId: resolveClientId(+dest, parent_order_id),
        accountId: accountId,
        sourceId: sourceId,
    
        source: source,
        dest: dest,

        tax: toDecimal(+item.tax, 3) || '--',
        commission: toDecimal(+item.commission, 3) || '--'
    }
}


export const dealPos = (item: PosOriginData): PosData => {
    const { update_time } = item;
    //item.type :'0': 未知, '1': 股票, '2': 期货, '3': 债券
    const direction: string = PosDirection[item.direction] || '--';
    const avgPrice: number = item.avg_open_price || item.position_cost_price || 0;
    return {
        updateTime: kungfu.formatTime(update_time, '%H:%M:%S.%N').slice(0, 12),
        updateTimeMMDD: kungfu.formatTime(update_time, '%m/%d %H:%M:%S.%N').slice(0, 18),
        updateTimeNum: +Number(update_time || 0),

        id: item.instrument_id + direction,
        instrumentId: item.instrument_id,
        instrumentType: item.instrument_type,
        exchangeId: item.exchange_id,

        direction,
        directionOrigin: item.direction,

        yesterdayVolume: Number(item.yesterday_volume) || 0,
        todayVolume: Number(item.volume) - Number(item.yesterday_volume) || 0,
        totalVolume: Number(item.volume) || 0, 

        avgPrice: avgPrice || 0,
        lastPrice: item.last_price || 0,
        totalPrice: +avgPrice * Number(item.volume) || 0,
        totalMarketPrice: item.last_price * Number(item.volume) || 0,
        unRealizedPnl: item.unrealized_pnl || 0,
        
        accountId: item.account_id,
        sourceId: item.source_id,
        clientId: item.client_id,

        ledgerCategory: item.ledger_category,
        accountIdResolved: `${item.source_id}_${item.account_id}`
    }
}


export const dealAsset = (item: AssetOriginData): AssetData => {
    return {
        accountIdResolved: `${item.source_id}_${item.account_id}`,
        accountId: item.account_id,
        sourceId: item.source_id,
        clientId: item.client_id,
        ledgerCategory: item.ledger_category,

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

export const dealSnapshot = (item: AssetSnapshotOriginData): AssetSnapshotData => {
    const { update_time } = item;
    return {
        updateTime: kungfu.formatTime(update_time, '%H:%M'),
        updateTimeMMDD: kungfu.formatTime(update_time, '%m/%d'),
        updateTimeNum: +Number(update_time || 0),
        tradingDay: item.trading_day,
        ledgerCategory: item.ledger_category,

        sourceId: item.source_id,
        brokerId: item.broker_id,
        accountId: item.account_id,
        clientId: item.client_id,
    
        initialEquity: item.initial_equity, //期初权益
        staticEquity: item.static_equity, //静态权益
        dynamicEquity: item.dynamic_equity, //动态权益
        realizedPnl: item.realized_pnl, //累计收益
        unrealizedPnl: item.unrealized_pnl,
        avail: item.avail,        //可用资金
        marketValue: item.market_value, //市值(股票)
        margin: item.margin, //保证金(期货)
        accumulatedFee: item.accumulated_fee, //累计手续费
        intradayFee: item.intraday_fee,   //当日手续费
        frozenCash: item.frozen_cash,   //冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
        frozenMargin: item.frozen_margin, //冻结保证金(期货)
        frozenFee: item.frozen_fee,    //冻结手续费(期货)
        positionPnl: item.position_pnl, //持仓盈亏(期货)
        closePnl: item.close_pnl    //平仓盈亏(期货)Í
    }
}

export const dealOrderStat = (item: OrderStatOriginData | null): OrderStatData | {} => {

    if (!item) return {};

    const { insert_time, ack_time, md_time, trade_time } = item;
    const latencyTrade = trade_time ? +toDecimal(Number(trade_time - ack_time) / 1000) : 0;
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


export const dealQuote = (quote: QuoteOriginData): QuoteData => {
    return {
        id: quote.exchange_id + quote.source_id + quote.instrument_id.toString() + String(quote.data_time),
        dataTime: kungfu.formatTime(quote.data_time, '%Y-%m-%d %H:%M:%S'),
        dataTimeNumber: quote.data_time.toString(),
        
        sourceId: quote.source_id,
        exchangeId: quote.exchange_id,
        instrumentId: quote.instrument_id,
        instrumentType: InstrumentType[quote.instrument_type],
        instrumentTypeOrigin: quote.instrument_type,

        highPrice: toDecimal(ensureNum(quote.high_price), 3),
        closePrice: toDecimal(ensureNum(quote.close_price), 3),
        lastPrice: toDecimal(ensureNum(quote.last_price), 3),
        lowPrice: toDecimal(ensureNum(quote.low_price), 3),
        lowerLimitPrice: toDecimal(ensureNum(quote.lower_limit_price), 3),
        openInterest: ensureNum(quote.open_interest),
        openPrice: toDecimal(ensureNum(quote.open_price), 3),
        preClosePrice: toDecimal(ensureNum(quote.pre_close_price), 3),
        preOpenInterest: ensureNum(quote.pre_open_interest),
        preSettlementPrice: toDecimal(ensureNum(quote.pre_settlement_price), 3),
        settlementPrice: toDecimal(ensureNum(quote.settlement_price), 3),
 

        tradingDay: quote.trading_day,
        turnover: ensureNum(quote.turnover),
        upperLimitPrice: toDecimal(ensureNum(quote.upper_limit_price), 3),
        volume: ensureNum(quote.volume),
        askPrices: quote.ask_price.map((num: number) => toDecimal(ensureNum(num), 3)) || [],
        askVolumes: quote.ask_volume.map((num: BigInt) => num.toString()) || [],
        bidPrices: quote.bid_price.map((num: number) => toDecimal(ensureNum(num), 3)) || [],
        bidVolumes: quote.bid_volume.map((num: BigInt) => num.toString()) || [],
    }
       

}



// ========================== 交易数据处理 end ===========================

