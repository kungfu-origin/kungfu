import moment from 'moment';
import { InstrumentTypes, aliveOrderStatusList, ExchangeIds, SideName, OffsetName, PosDirection } from 'kungfu-shared/config/tradingConfig';

export const getCurrentCount = ({
    currentTimestamp, 
    deltaMilliSeconds,
    finishTime,
    LOOP_INTERVAL,
    LAST_SINGULARITY_SECOND,
    LAST_STEP_COUNT
}: {
    currentTimestamp: number; 
    deltaMilliSeconds: number;
    finishTime: number;
    LOOP_INTERVAL: number;
    LAST_SINGULARITY_SECOND: number;
    LAST_STEP_COUNT: number
}) => {
    const deltaCurrentToFinishTime = finishTime - currentTimestamp;
    const currentCount = Math.floor(deltaMilliSeconds / LOOP_INTERVAL)
    if (LAST_SINGULARITY_SECOND) {
        if (deltaCurrentToFinishTime <= LAST_SINGULARITY_SECOND) {
            return LAST_STEP_COUNT
        } else {
            if (currentCount === LAST_STEP_COUNT) {
                return currentCount - 1
            }
        }
    }
    return currentCount
};

export const reqTimer = (fn: Function, interval: number) => {
    fn()
    return setInterval(() => {
        fn()
    }, interval)
}

export const getCurrentTimestamp = (format = false): number | string => {
    if (format) {
        return moment().format('HH:mm:ss.SSS')
    }
    return +new Date().getTime()
}

export const transformArrayToObjectByKey = (targetList: Array<any>, keys: Array<string>): any => {
    let data: any = {};
    (targetList || []).forEach(item => {
        const key:string = keys.map(k => item[k]).join('_')
        data[key] = item
    })

    return data
}

export const getTickerNamesByTicker = (ticker: string) => {
    const len = ticker.length
    const target = ticker.slice(0, len - 4)
    const tm = ticker.slice(len - 2)
    const ty = ticker.slice(len - 4, len - 2)
    const nm = +tm + 1 > 12 ? 1 : +tm + 1
    const ny = +tm + 1 > 12 ? +ty + 1: ty
    const tsm = getMonth(+tm).tsm;
    const nsm = getMonth(+tm).nsm;
    return {
        tm: `${target}${keepDateLen(ty)}${keepDateLen(tm)}`,
        nm: `${target}${keepDateLen(ny)}${keepDateLen(nm)}`,
        tsm: `${target}${keepDateLen(ny)}${keepDateLen(tsm)}`,
        nsm: `${target}${keepDateLen(ny)}${keepDateLen(nsm)}`,
    }
}

export const reqMakeOrder = (baseData: any, quote: QuoteData) => {
    const { side, offset, accountId, volume, parentId, sim } = baseData;
    const { instrumentTypeOrigin, instrumentId, exchangeId } = quote;

    const makeOrderPrice = getMakeOrderPrice(side, quote)

    const makeOrderData = { 
        name: accountId,
        instrument_id: instrumentId,
        instrument_type: +instrumentTypeOrigin,
        exchange_id: exchangeId,
        limit_price: makeOrderPrice,
        volume,
        side: side,
        offset: offset,
        price_type: 0,
        hedge_flag: 0,
        parent_id: parentId
    }

    if (!ensureNum(makeOrderPrice)) {
        console.log(`[ERROR] 下单信息错误 ${JSON.stringify(quote)}`)
        return false;
    }

    if (!sim) {
        //@ts-ignore
        process.send({
            type: 'process:msg',
            data: {
                type: 'MAKE_ORDER_BY_PARENT_ID',
                body: {
                    ...makeOrderData
                }
            }
        })
    } else {
        console.log('--------- [模拟] ---------')
    }
   
    console.log(`--------- [下单] ---------`)
    console.log(`[账户] ${makeOrderData.name}`)
    console.log(`[标的] ${makeOrderData.instrument_id}`)
    console.log(`[交易所] ${ExchangeIds[makeOrderData.exchange_id]}`)
    console.log(`[交易数量] ${makeOrderData.volume}`)
    console.log(`[买卖] ${SideName[makeOrderData.side]}`)
    console.log(`[开平] ${OffsetName[makeOrderData.offset]}`)
    console.log(`[价格] ${makeOrderData.limit_price}`)
    console.log(`[订单组] ${makeOrderData.parent_id}`)
    console.log(`---------------------------`)

    return makeOrderData
}

export const getTickerWithMaxValue = (targetList: Array<any>, targetKey: string) => {
    const afterSort = (targetList || []).sort((a: any, b: any) => {
        const vA = +a[targetKey] || 0;
        const vB = +b[targetKey] || 0;
        return vB - vA;
    })

    return afterSort[0]
}

export const getTickerWithMinValue = (targetList: Array<any>, targetKey: string) => {
    const afterSort = (targetList || []).sort((a: any, b: any) => {
        const vA = +a[targetKey] || 0;
        const vB = +b[targetKey] || 0;
        return vA - vB;
    })

    return afterSort[0]
}

export const ensureTargetIncludesAllKeys = (targetObject: any, keys: Array<string>) => {
    let flag = true;

    keys.forEach(key => {
        if (targetObject[key] === undefined) {
            flag = false;
        }
    })

    return flag;
}

export function ensureNum (num: number | bigint | string) {
    num = +(num.toString());
    if (Number.isNaN(num)) return 0
    if (!Number.isFinite(num)) return 0
    if (num === 1.7976931348623157e+308) return 0
    return +num
}

function getMonth (m: number) {
    const ms = [3, 6, 9, 12, 3]
    const tsm = ms.filter(sm => sm >= m)[0]
    const nsm = ms[ms.indexOf(tsm) + 1]
    return {
        tsm, nsm
    }
}

function keepDateLen (n: string | number) {
    return +n < 10 ? `0${+n}` : `${+n}`
} 


function getMakeOrderPrice (side: number, quote: QuoteData ) {
    const { lastPrice, askPrices, bidPrices } = quote;
    
    if (+side === 0) {
        return askPrices[0]
    } else if (+side === 1) {
        return bidPrices[0]
    }

    return lastPrice
}

export function recordTaskInfo (calculatedData: any, tradeData: any, globalData: any) {
    const postData = {
        mode: globalData.sim ? '模拟' : '实盘',
        updateTime: +new Date().getTime(),
        instrumentId: calculatedData.name,
        instrumentPrice: calculatedData.instrumentPrice,
        indexId: calculatedData.indexId,
        indexPrice: calculatedData.indexP,
        backwardsDelta: calculatedData.backwardsDelta,
        backWardsRatio: calculatedData.backWardsRatio,
        backWardsRatioByYears: calculatedData.backwardByYear,
        expiredDate: calculatedData.expireDate,
        remainDays: calculatedData.toExpireDate,
        side: tradeData ? SideName[tradeData.side] : '',
        offset: tradeData ? OffsetName[tradeData.offset] : '',
        limitPrice: tradeData ? tradeData.limit_price : '',
        volume: tradeData ? tradeData.volume : '',
        volumeLefted: tradeData ? `${globalData.volume - globalData.tradedVolume} / ${globalData.volume}` : '',
        accountId: tradeData ? tradeData.name : ''
    }

      //@ts-ignore
      process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_RECORD_DATA',
            body: {
                mode: globalData.sim ? 'sim' : 'real',
                data: {
                    ...postData                    
                }
            }
        }
    })
}
