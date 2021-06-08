import moment from 'moment';
import { ExchangeIds, SideName, OffsetName } from '@kungfu-trader/kungfu-shared/config/tradingConfig';



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
        const key:string = keys.map(k => item[k].toString().trim()).filter(k => !!k).join('_')
        if (key) {
            data[key] = item
        }
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

export function recordTaskInfo (calculatedData: any, globalData: any) {
    const postData = {
        updateTime: +new Date().getTime(),
        instrumentId: calculatedData.name,
        instrumentPrice: calculatedData.instrumentPrice,
        indexId: calculatedData.indexId,
        indexPrice: calculatedData.indexP,
        backwardsDelta: calculatedData.backwardsDelta,
        backwardsRatio: Number(calculatedData.backwardsRatio * 100).toFixed(2) + '%',
        backwardsRatioByYears: Number(calculatedData.backwardByYear * 100).toFixed(2) + '%',
        expiredDate: calculatedData.expireDate,
        remainDays: calculatedData.toExpireDate,
        side: SideName[globalData.side],
        selected: calculatedData.selected ? '✓' : '',
    }

      //@ts-ignore
      process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_RECORD_DATA',
            body: {
                data: {
                    ...postData                    
                }
            }
        }
    })
}
