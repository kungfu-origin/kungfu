import dayjs from 'dayjs';
import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
    dealTradingData,
    filterLedgerResult,
    getOrderTradeFilterKey,
    getTradingDataSortKey,
    kfLogger,
} from '../utils/busiUtils';
import { HistoryDateEnum, KfConfig, KfLocation } from '../typings';

export const kf = kungfu();
kfLogger.info('Load kungfu node');

export const configStore = kf.ConfigStore(KF_RUNTIME_DIR);
export const history = kf.History(KF_RUNTIME_DIR);

export const dealKfTime = (nano: bigint, date = false): string => {
    if (nano === BigInt(0)) {
        return '--';
    }

    if (date) {
        return kf.formatTime(nano, '%m/%d %H:%M:%S');
    }
    return kf.formatTime(nano, '%H:%M:%S');
};

export const dealOrderStat = (
    ledger: TradingData | undefined,
    orderUKey: string,
): {
    latencySystem: string;
    latencyNetwork: string;
    latencyTrade: string;
    trade_time: bigint;
} | null => {
    if (!ledger) {
        return null;
    }

    const orderStat = ledger.OrderStat[orderUKey];
    if (!orderStat) {
        return null;
    }

    const { insert_time, ack_time, md_time, trade_time } = orderStat;
    const latencyTrade =
        trade_time && ack_time
            ? Number(Number(trade_time - ack_time) / 1000).toFixed(0)
            : '--';
    const latencyNetwork =
        ack_time && insert_time
            ? Number(Number(ack_time - insert_time) / 1000).toFixed(0)
            : '--';
    const latencySystem =
        insert_time && md_time
            ? Number(Number(insert_time - md_time) / 1000).toFixed(0)
            : '--';

    return {
        latencySystem,
        latencyNetwork,
        latencyTrade,
        trade_time: orderStat.trade_time,
    };
};

export const getKungfuDataByDateRange = (
    date: number | string,
    dateType = HistoryDateEnum.naturalDate, //0 natural date, 1 tradingDate
): Promise<TradingData | Record<string, unknown>> => {
    const tradingDay = dayjs(date).format('YYYYMMDD');
    const from = dayjs(date).format('YYYY-MM-DD');
    const to = dayjs(date).add(1, 'day').format('YYYY-MM-DD');
    const yesFrom = dayjs(date).add(-1, 'day').format('YYYY-MM-DD');
    const fridayFrom = dayjs(date).add(-3, 'day').format('YYYY-MM-DD');
    const fridayTo = dayjs(date).add(-2, 'day').format('YYYY-MM-DD');
    const dataTypeForHistory: TradingDataTypeName[] = [
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
                const historyData: TradingData | Record<string, unknown> = {};
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
                            kungfuDataFriday[key as keyof TradingData],
                            kungfuDataYesterday[key as keyof TradingData],
                            kungfuDataToday[key as keyof TradingData],
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
    watcher: Watcher | null,
    date: string,
    dateType: HistoryDateEnum,
    tradingDataTypeName: TradingDataTypeName,
    kfLocation?: KfLocation | KfConfig,
) => {
    return getKungfuDataByDateRange(date, dateType).then(
        (tradingData: TradingData | Record<string, unknown>) => {
            if (!kfLocation) {
                //all
                return;
            }

            return dealTradingData(
                watcher,
                tradingData as TradingData,
                tradingDataTypeName,
                kfLocation,
            );
        },
    );
};
