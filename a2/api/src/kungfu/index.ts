import dayjs from 'dayjs';
import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
    dealDirection,
    dealHedgeFlag,
    dealInstrumentType,
    dealLocationUID,
    dealOffset,
    dealOrderStatus,
    dealSide,
    dealTradingData,
    getIdByKfLocation,
    getOrderTradeFilterKey,
    kfLogger,
} from '../utils/busiUtils';
import { HistoryDateEnum } from '../typings/enums';

export const kf = kungfu();
kfLogger.info('Load kungfu node');

export const configStore = kf.ConfigStore(KF_RUNTIME_DIR);
export const history = kf.History(KF_RUNTIME_DIR);
export const longfist = kf.longfist;

export const dealKfTime = (nano: bigint, date = false): string => {
    if (nano === BigInt(0)) {
        return '--';
    }

    if (date) {
        return kf.formatTime(nano, '%m/%d %H:%M:%S');
    }
    return kf.formatTime(nano, '%H:%M:%S');
};

export const dealTradingDataItem = (
    item: KungfuApi.TradingDataTypes,
    watcher: KungfuApi.Watcher | null,
) => {
    const itemResolved = { ...item } as Record<string, unknown>;
    if ('trade_time' in item) {
        itemResolved.trade_time = dealKfTime(item.trade_time, true);
    }
    if ('insert_time' in item) {
        itemResolved.insert_time = dealKfTime(item.insert_time, true);
    }
    if ('update_time' in item) {
        itemResolved.update_time = dealKfTime(item.update_time, true);
    }
    if ('direction' in item) {
        itemResolved.direction = dealDirection(item.direction).name;
    }
    if ('side' in item) {
        itemResolved.side = dealSide(item.side).name;
    }
    if ('offset' in item) {
        itemResolved.offset = dealOffset(item.offset).name;
    }
    if ('status' in item && 'error_msg' in item) {
        itemResolved.status = dealOrderStatus(
            item.status,
            item.error_msg || '',
        );
    }
    if ('instrument_type' in item) {
        itemResolved.instrument_type = dealInstrumentType(
            item.instrument_type,
        ).name;
    }

    if ('hedge_flag' in item) {
        itemResolved.hedge_flag = dealHedgeFlag(item.hedge_flag).name;
    }
    if ('source' in item && watcher) {
        itemResolved.source = dealLocationUID(watcher, item.source);
    }
    if ('dest' in item && watcher) {
        itemResolved.dest = dealLocationUID(watcher, item.dest);
    }
    if ('holder_uid' in item && watcher) {
        itemResolved.holder_uid = dealLocationUID(watcher, item.holder_uid);
    }

    return itemResolved;
};

export const getKungfuDataByDateRange = (
    date: number | string,
    dateType = HistoryDateEnum.naturalDate, //0 natural date, 1 tradingDate
): Promise<KungfuApi.TradingData | Record<string, unknown>> => {
    const tradingDay = dayjs(date).format('YYYYMMDD');
    const from = dayjs(date).format('YYYY-MM-DD');
    const to = dayjs(date).add(1, 'day').format('YYYY-MM-DD');
    const yesFrom = dayjs(date).add(-1, 'day').format('YYYY-MM-DD');
    const fridayFrom = dayjs(date).add(-3, 'day').format('YYYY-MM-DD');
    const fridayTo = dayjs(date).add(-2, 'day').format('YYYY-MM-DD');
    const dataTypeForHistory: KungfuApi.TradingDataTypeName[] = [
        'Order',
        'Trade',
        'OrderStat',
        'Position',
        'Asset',
        'OrderInput',
    ];

    return new Promise((resolve) => {
        const timer = setTimeout(() => {
            //by trading date
            if (dateType === HistoryDateEnum.tradingDate) {
                const kungfuDataToday = history.selectPeriod(from, to);
                const kungfuDataYesterday = history.selectPeriod(yesFrom, from);
                const kungfuDataFriday = history.selectPeriod(
                    fridayFrom,
                    fridayTo,
                );
                const historyData:
                    | KungfuApi.TradingData
                    | Record<string, unknown> = {};
                dataTypeForHistory.forEach((key) => {
                    if (
                        key === 'Order' ||
                        key === 'Trade' ||
                        key === 'OrderInput'
                    ) {
                        historyData[key] = Object.assign(
                            kungfuDataToday[key].filter(
                                'trading_day',
                                tradingDay,
                            ),
                            kungfuDataYesterday[key].filter(
                                'trading_day',
                                tradingDay,
                            ),
                            kungfuDataFriday[key].filter(
                                'trading_day',
                                tradingDay,
                            ),
                        );
                    } else {
                        historyData[key] = Object.assign(
                            kungfuDataFriday[
                                key as keyof KungfuApi.TradingData
                            ],
                            kungfuDataYesterday[
                                key as keyof KungfuApi.TradingData
                            ],
                            kungfuDataToday[key as keyof KungfuApi.TradingData],
                        );
                    }
                });

                resolve(historyData);
            } else {
                const kungfuDataToday = history.selectPeriod(from, to);
                resolve(kungfuDataToday);
            }
            clearTimeout(timer);
        }, 160);
    });
};

export const getKungfuHistoryData = (
    watcher: KungfuApi.Watcher | null,
    date: string,
    dateType: HistoryDateEnum,
    tradingDataTypeName: KungfuApi.TradingDataTypeName | 'all',
    kfLocation?: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<{
    tradingData: KungfuApi.TradingData;
    historyDatas: KungfuApi.TradingDataTypes[];
}> => {
    return getKungfuDataByDateRange(date, dateType).then(
        (tradingData: KungfuApi.TradingData | Record<string, unknown>) => {
            if (tradingDataTypeName === 'all') {
                return {
                    tradingData: tradingData as KungfuApi.TradingData,
                    historyDatas: [],
                };
            }

            if (!kfLocation) {
                return {
                    tradingData: tradingData as KungfuApi.TradingData,
                    historyDatas: [],
                };
            }

            return {
                tradingData: tradingData as KungfuApi.TradingData,
                historyDatas: dealTradingData(
                    watcher,
                    tradingData as KungfuApi.TradingData,
                    tradingDataTypeName,
                    kfLocation,
                ),
            };
        },
    );
};

export const kfRequestMarketData = (
    watcher: KungfuApi.Watcher | null,
    exchangeId: string,
    instrumentId: string,
    mdLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void> => {
    if (!watcher) {
        return Promise.reject(new Error(`Watcher 错误`));
    }

    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 进程未连接`));
    }

    if (!watcher.isReadyToInteract(mdLocation)) {
        const sourceId = getIdByKfLocation(mdLocation);
        return Promise.reject(new Error(`行情源 ${sourceId} 未就绪`));
    }

    return Promise.resolve(
        watcher.requestMarketData(mdLocation, exchangeId, instrumentId),
    );
};

export const kfCancelOrder = (
    watcher: KungfuApi.Watcher | null,
    orderId: bigint,
    tdLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
    strategyLocation?: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void> => {
    if (!watcher) {
        return Promise.reject(new Error(`Watcher 错误`));
    }

    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接`));
    }

    if (!watcher.isReadyToInteract(tdLocation)) {
        const accountId = getIdByKfLocation(tdLocation);
        return Promise.reject(new Error(`交易账户 ${accountId} 未就绪`));
    }

    const orderAction: KungfuApi.OrderAction = {
        ...longfist.OrderAction(),
        order_id: orderId,
    };

    if (strategyLocation) {
        return Promise.resolve(
            watcher.cancelOrder(orderAction, tdLocation, strategyLocation),
        );
    } else {
        return Promise.resolve(watcher.cancelOrder(orderAction, tdLocation));
    }
};

export const kfCancelAllOrders = (
    watcher: KungfuApi.Watcher | null,
    kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void[]> => {
    if (!watcher) {
        return Promise.reject(new Error(`Watcher 错误`));
    }

    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接`));
    }

    if (kfLocation.category !== 'td' && kfLocation.category !== 'strategy') {
        return Promise.reject(new Error(`Category 错误 仅支持td strategy`));
    }

    if (
        kfLocation.category === 'td' &&
        !watcher.isReadyToInteract(kfLocation)
    ) {
        const accountId = getIdByKfLocation(kfLocation);
        return Promise.reject(new Error(`交易账户 ${accountId} 未就绪`));
    }

    const filterKey = getOrderTradeFilterKey(kfLocation.category);
    const orders = watcher.ledger.Order.filter(
        filterKey,
        watcher.getLocationUID(kfLocation),
    ).list();
    const cancelOrderTasks = orders.map(
        (item: KungfuApi.Order): Promise<void> => {
            const orderAction: KungfuApi.OrderAction = {
                ...longfist.OrderAction(),
                order_id: item.order_id,
            };
            if (kfLocation.category === 'td') {
                return Promise.resolve(
                    watcher.cancelOrder(orderAction, kfLocation),
                );
            } else if (kfLocation.category === 'strategy') {
                return Promise.resolve(
                    watcher.cancelOrder(
                        orderAction,
                        watcher.getLocation(item.source),
                        kfLocation,
                    ),
                );
            } else {
                return Promise.resolve();
            }
        },
    );

    return Promise.all(cancelOrderTasks);
};

export const kfMakeOrder = (
    watcher: KungfuApi.Watcher | null,
    makeOrderInput: KungfuApi.MakeOrderInput,
    tdLocation: KungfuApi.KfLocation,
    strategyLocation?: KungfuApi.KfLocation,
): Promise<void> => {
    if (!watcher) {
        return Promise.reject(new Error('Watcher 错误'));
    }

    if (!watcher.isLive()) {
        return Promise.reject(new Error(`Master 未连接`));
    }

    if (!watcher.isReadyToInteract(tdLocation)) {
        const accountId = getIdByKfLocation(tdLocation);
        return Promise.reject(new Error(`交易账户 ${accountId} 未就绪`));
    }

    const now = watcher.now();
    const orderInput: KungfuApi.OrderInput = {
        ...longfist.OrderInput(),
        ...makeOrderInput,
        limit_price: makeOrderInput.limit_price || 0,
        frozen_price: makeOrderInput.limit_price || 0,
        volume: BigInt(makeOrderInput.volume),
        insert_time: now,
    };

    if (strategyLocation) {
        //设置orderInput的parentid，来标记该order为策略手动下单
        return Promise.resolve(
            watcher.issueOrder(
                {
                    ...orderInput,
                    parent_id: now,
                },
                tdLocation,
                strategyLocation,
            ),
        );
    } else {
        return Promise.resolve(watcher.issueOrder(orderInput, tdLocation));
    }
};

export const hashInstrumentUKey = (
    instrumentId: string,
    exchangeId: string,
): string => {
    return (BigInt(kf.hash(instrumentId)) ^ BigInt(kf.hash(exchangeId)))
        .toString(16)
        .padStart(16, '0');
};
