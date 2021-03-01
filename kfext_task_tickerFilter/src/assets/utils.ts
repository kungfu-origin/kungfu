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
    const { side, offset, accountId, volume, parentId } = baseData;
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
}

export const getTickerWithMaxValue = (com: { [prop: string]: number }) => {
    const values = Object.values(com);
    const keys = Object.keys(com);
    const maxV = Math.max(...values);
    const index = values.indexOf(maxV);
    const name = keys[index];
    return {
        name,
        value: maxV
    }
}

export const getTickerWithMinValue = (com: { [prop: string]: number }) => {
    const values = Object.values(com);
    const keys = Object.keys(com);
    const minV = Math.min(...values);
    const index = values.indexOf(minV);
    const name = keys[index];
    return {
        name,
        value: minV
    }
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