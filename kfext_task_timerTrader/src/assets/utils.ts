
import moment from 'moment';
import { InstrumentTypes, aliveOrderStatusList, ExchangeIds, SideName, OffsetName, PosDirection } from 'kungfu-shared/config/tradingConfig';

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

export const makeOrderDirectionType = (side: number, offset: number): direcData => {
    if (+side === 0) {
        if (+offset === 0) {
            return { d: 0, n: '买开', dc: 1} // 'long-open'
        } else if (+offset === 1) {
            return { d: 1, n: '买平', dc: 0} // 'short-close'
        }
    } else if (+side === 1) {
        if (+offset === 0) {
            return { d: 1, n: '卖开', dc: 0 } // 'short-open'
        } else if (+offset === 1) {
            return { d: 0, n: '卖平', dc: 1 } // 'long-close'
        }
    }

    return {
        d: 0,
        n: '未知',
        dc: 1
    }
}

export const buildTarget = ({ offset, side, ticker, totalVolume, totalVolumeCont, targetVolume }: { 
    offset: number,
    side: number,
    ticker: string,
    totalVolume: number,
    totalVolumeCont: number,
    targetVolume: number
}): VolumeRecordData[] | Boolean => {
    //0 long 1 short;
    if (+side === 0) {
        if (+offset === 0) {
            console.log(`[TARGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标买开 ${targetVolume}`)    
            return [
                { d: 0, v: totalVolume + targetVolume }
            ]            
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TARGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}，现有持仓不足，需买平 ${totalVolume}，买开 ${delta}`)                
                return [
                    { d: 1, v: 0 },
                    { d: 0, v: delta + totalVolumeCont }
                ]
            } else {
                console.log(`[TARGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}`)    
                return [
                    { d: 1, v: totalVolume - targetVolume },
                ]
            }
        }
    } else if (+side === 1) {
        if (+offset === 0) {
            console.log(`[TARGET] 标的 ${ticker}，现有空仓${totalVolume}，目标卖开${targetVolume}`)      
            return [
                { d: 1, v: totalVolume + targetVolume },
            ]
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TARGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}，现有持仓不足，需卖平 ${totalVolume}，卖开 ${delta}`)                
                return [
                    { d: 0, v: 0 },
                    { d: 1, v: delta + totalVolumeCont }
                ]   
            } else {
                console.log(`[TARGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}`)                
                return [
                    { d: 0, v: totalVolume - targetVolume },
                ]
            }
        }
    }

    return false
}



export const reqMakeOrder = (baseData: any, quote: QuoteData, unfinishedSteps: number) => {
    const { side, offset, accountId, volume, parentId, priceMode } = baseData;
    const { instrumentTypeOrigin, instrumentId, exchangeId } = quote;

    const makeOrderPrice = getMakeOrderPrice(side, quote, unfinishedSteps, priceMode)

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
            
    return makeOrderData
            
}

function getMakeOrderPrice (side: number, quote: QuoteData, unfinishedSteps: number, priceMode: number ) {
    const { upperLimitPrice, lowerLimitPrice, lastPrice, askPrices, bidPrices } = quote;
    
    if (+side === 0) {
        if (unfinishedSteps > 1) {
            if (+priceMode === 0) {
                return askPrices[0]
            } else if (+priceMode === 1) {
                return bidPrices[0]
            } else {
                return lastPrice
            }
        } else {
            return upperLimitPrice
        }
    } else if (+side === 1) {
        if (unfinishedSteps > 1) {
            if (+priceMode === 0) {
                return bidPrices[0]
            } else if (+priceMode === 1) {
                return askPrices[0]                
            } else {
                return lastPrice
            }
        } else {
            return lowerLimitPrice
        }
    }

    return lastPrice
}

export const reqCancelOrder = (parentId: string) => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'CANCEL_ORDER_BY_PARENT_ID',
            body: {
                parentId
            }
        }
    })
    console.log(`[撤单] PARENTID: ${parentId}`)        
}

// stock 最小下单单位为100
function dealMakeOrderVolume (instrumentType: number, volume: number): number {
    //stock 100的倍数
    const scale100 = Object.keys(InstrumentTypes)
        .filter(key => {
            if (key.toLowerCase().includes('stock')) return true;
            return false;
        })
        .map(key => +InstrumentTypes[key])

    const isStock = scale100.includes(+instrumentType);
    
    if (isStock) {
        const scale = +Number(volume / 100).toFixed(0)
        const scaleResolved = scale < 1 ? 1 : scale;
        return scaleResolved * 100
    }

    return +Number(volume).toFixed(0)
}

export const getAliveOrders = (orders: OrderData[]) => {
    return orders
    .map(order => {
        const { status, orderId } = order;
        return { orderId, status }
    })
    .filter(order => {
        const { status } = order;
        if (aliveOrderStatusList.includes(+status)) {
            return true;
        }
        return false
    })
}

export const timeCheckBySecond = (currentSecond: number, quote: QuoteData) => {
    if (currentSecond >= 0) return
    const secondResolved = Math.abs(currentSecond);

    if ((+secondResolved <= 10) || (+secondResolved === 120) || (+secondResolved === 60)) {
        console.log(`[倒计时] ${secondResolved}s`)
    }

    if (+secondResolved === 120) {
        //@ts-ignore
        process.send({
            type: 'process:msg',
            data: {
                type: 'TIME_ALERT',
                body: {
                    minute: '2',
                    quoteAlive: !!quote
                }
            }
        })
    }

    if (+secondResolved === 60) {
        //@ts-ignore
        process.send({
            type: 'process:msg',
            data: {
                type: 'TIME_ALERT',
                body: {
                    minute: '1',
                    quoteAlive: !!quote,
                    quote
                }
            }
        })
    }
}

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

export const calcVolumeThisStep = (
        positions: StringToPosData, 
        TICKER: string, 
        TARGET_DIRECTION: number, 
        TARGET_DIRECTION_CONT: number, 
        offset: number,
        targetPosData: VolumeRecordData[], 
        unfinishedSteps: number, 
        instrumentType: number
    ) => {
    const pos = positions[`${TICKER}_${TARGET_DIRECTION}`] || {};
    const posCont = positions[`${TICKER}_${TARGET_DIRECTION_CONT}`] || {};
    const currentVolume = +pos.totalVolume || 0;
    const currentVolumeCont = +posCont.totalVolume || 0;
    const yesterdayVolume = +pos.yesterdayVolume || 0;
    const todayVolume = +pos.todayVolume || 0;

    const currentVolumeData: any = {
        [+TARGET_DIRECTION]: currentVolume,
        [+TARGET_DIRECTION_CONT]: currentVolumeCont
    }

    const totalTargetVolume: number = targetPosData
        .map((item: VolumeRecordData): number => {
            const { d, v } = item;
            const currentV = currentVolumeData[d];
            const delta = currentV - v;
            //需要根据开平方向判断， 开->大于目标仓位->0，平->小于目标仓位->0
            if (+d === +TARGET_DIRECTION) {
                if (+offset === 0) {
                    return delta > 0 ? 0 : Math.abs(delta)
                } else {
                    return delta < 0 ? 0 : Math.abs(delta)
                }
            } else {
                //此时真实应用offset为反向
                if (+offset === 0) {
                    return delta < 0 ? 0 : Math.abs(delta)
                } else {
                    return delta > 0 ? 0 : Math.abs(delta)
                }
            }
        
        }) 
        .reduce((delta1: number, delta2: number) => {
            return +delta1 + +delta2
        }) || 0;

    const targetVolumeByStep = unfinishedSteps === 1 ? totalTargetVolume : totalTargetVolume / unfinishedSteps

    return {
        currentVolume,
        currentYesVolume: yesterdayVolume,
        currentTodayVolume: todayVolume,
        currentVolumeCont,
        total: totalTargetVolume,
        thisStepVolume: dealMakeOrderVolume(instrumentType, targetVolumeByStep)
    }
}

export const buildTradeTaskVolumeOffset = ({
    ticker,
    side,
    offset,
    currentVolume,
    currentYesVolume,
    currentTodayVolume,
    currentVolumeCont,
    thisStepVolume,
    total,
    OPERATION_NAME,
    TARGET_DIRECTION,
    TARGET_DIRECTION_CONT
}: {
    ticker: string;
    side: number;
    offset: number;
    currentVolume: number;
    currentYesVolume: number;
    currentTodayVolume: number;
    currentVolumeCont: number;
    thisStepVolume: number;
    total: number;
    OPERATION_NAME: string;
    TARGET_DIRECTION: number;
    TARGET_DIRECTION_CONT: number;
}) => {

    //deltaVolume 平仓计算目前仓位是否足够，小于0，足够，大于0，不够，需要反向
    const deltaVolume = getDeltaVolume(offset, thisStepVolume, currentVolume)
    buildTradeTaskLog(ticker, side, currentVolume, currentYesVolume, currentTodayVolume, total, currentVolumeCont, deltaVolume, thisStepVolume, OPERATION_NAME, TARGET_DIRECTION, TARGET_DIRECTION_CONT)

    if (+thisStepVolume <= 0) return []
    
    if (offset === 0) { // 开
        return [{ offset: 0, volume: thisStepVolume }]

    } else { // 平
        let taskList: Array<TradeTarget> = [];

        //为了同时适配 现仓够与不够两种情况，持仓够，用thisStepVolume, 持仓不够，用currentVolume
        const targetCloseVolume = Math.min(thisStepVolume, currentVolume)

        //现仓不为0
        if (targetCloseVolume > 0) {
            if (targetCloseVolume <= currentYesVolume) { // 昨够
                taskList = [{ offset: 3, volume: targetCloseVolume }]
            } else if (currentYesVolume > 0) { // 昨今都有
                const thisStepTodayVolume = +Number(targetCloseVolume - currentYesVolume).toFixed(0);
                taskList = [{ offset: 3, volume: currentYesVolume }, { offset: 2, volume: thisStepTodayVolume }]
            } else {// 只有今
                taskList = [{ offset: 2, volume: targetCloseVolume }]
            }
        }

        if (+deltaVolume > 0) {
            taskList = [ ...taskList, { offset: 0, volume: deltaVolume }]
        }

        return taskList
    }
}

function getDeltaVolume (offset: number, thisStepVolume: number, currentVolume: number) {
    if (offset === 0) {
        return -1
    } else {
        return +Number(thisStepVolume - currentVolume).toFixed(0);
    }
}

function buildTradeTaskLog (
    ticker: string,
    side: number,
    currentVolume: number,
    currentYesVolume: number,
    currentTodayVolume: number,
    total: number,
    currentVolumeCont: number,
    deltaVolume: number,
    thisStepVolume: number,
    OPERATION_NAME: string,
    TARGET_DIRECTION: number,
    TARGET_DIRECTION_CONT: number
) {

    const contOperationName = makeOrderDirectionType(side, 0).n;
    const countPos = deltaVolume <= 0 ? '' : `${PosDirection[TARGET_DIRECTION_CONT]} ${currentVolumeCont}, `;
    const countOperation = deltaVolume <= 0 ? '' : `持仓不足, 需 ${OPERATION_NAME} ${currentVolume}, ${contOperationName} ${deltaVolume}`

    console.log(
        `现有 ${ticker}${PosDirection[TARGET_DIRECTION]} ${currentVolume}, ${countPos}
        其中 ${ticker}${PosDirection[TARGET_DIRECTION]} 昨 ${currentYesVolume}, 今 ${currentTodayVolume},
        还需 ${OPERATION_NAME} ${total}, 本次需 ${OPERATION_NAME} ${thisStepVolume}, 
        ${countOperation}`
    )
}

export function getCurrentTimestamp (format = false): number | string {
    if (format) {
        return moment().format('HH:mm:ss.SSS')
    }
    return +new Date().getTime()
}



export function getCancelOrderBeforeLastStepControllerTime (finishTime: number, lastStepInterval: number, lastestMakeOrdertimeStamp: number): number {
    return +Number(((finishTime - lastestMakeOrdertimeStamp - lastStepInterval) / 2) + lastStepInterval).toFixed(0)
}

export const printQuote = (quote: QuoteData): void => {
    if (quote) {
        console.log(`[行情价格]
        [标的] ${quote.instrumentId}
        [卖价] ${JSON.stringify(quote.askPrices)} 
        [买价] ${JSON.stringify(quote.bidPrices)}
        [涨停价格] ${quote.upperLimitPrice} 
        [跌停价格] ${quote.lowerLimitPrice} 
        [最高价] ${quote.highPrice}
        [最低价] ${quote.lowPrice}
        [最新价] ${quote.lastPrice}`)
    }
}

export function recordTaskInfo (quoteData: any, tradeData: any, globalData: any) {
    const bid1Price = (quoteData.bidPrices || [])[0]
    const bid1Volume = (quoteData.bidVolumes || [])[0]
    const ask1Price = (quoteData.askPrices || [])[0]
    const ask1Volume = (quoteData.askVolumes || [])[0]
    const postData = {
        updateTime: +new Date().getTime(),
        instrumentId: tradeData.name,
        lastPrice: quoteData.lastPrice,
        bid1PriceVolume: `${bid1Price}/${bid1Volume}`,
        ask1PriceVolume: `${ask1Price}/${ask1Volume}`,
        side: tradeData ? SideName[tradeData.side] : '',
        offset: tradeData ? OffsetName[tradeData.offset] : '',
        limitPrice: tradeData ? tradeData.limit_price : '',
        volume: tradeData ? tradeData.volume : '',
        volumeLefted: tradeData ? globalData.volumeLefted : '',
        count: globalData.count,
        accountId: tradeData ? tradeData.name : ''
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
