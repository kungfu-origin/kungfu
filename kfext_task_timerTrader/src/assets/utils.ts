
import { InstrumentTypes, aliveOrderStatusList, ExchangeIds, SideName, OffsetName } from 'kungfu-shared/config/tradingConfig';

export const transformArrayToObjectByKey = (targetList: Array<any>, keys: Array<string>): any => {
    let data: any = {};
    (targetList || []).forEach(item => {
        const key:string = keys.map(k => item[k]).join('_')
        data[key] = item
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

export const buildTarget = ({ offset, side, ticker, totalVolume, targetVolume }: { 
    offset: number,
    side: number,
    ticker: string,
    totalVolume: number,
    targetVolume: number
}): VolumeRecordData[] | Boolean => {
    if (+side === 0) {
        if (+offset === 0) {
            console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标买开 ${targetVolume}`)    
            return [
                { d: 0, v: totalVolume + targetVolume }
            ]            
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TRAGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}，现有持仓不足，需买平 ${totalVolume}，买开 ${delta}`)                
                return [
                    { d: 1, v: 0 },
                    { d: 0, v: delta }
                ]
            } else {
                console.log(`[TRAGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}`)    
                return [
                    { d: 1, v: totalVolume - targetVolume },
                ]
            }
        }
    } else if (+side === 1) {
        if (+offset === 0) {
            console.log(`[TRAGET] 标的 ${ticker}，现有空仓${totalVolume}，目标卖开${targetVolume}`)      
            return [
                { d: 1, v: totalVolume + targetVolume },
            ]
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}，现有持仓不足，需卖平 ${totalVolume}，卖开 ${delta}`)                
                return [
                    { d: 0, v: 0 },
                    { d: 1, v: delta }
                ]   
            } else {
                console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}`)                
                return [
                    { d: 0, v: totalVolume - targetVolume },
                ]
            }
        }
    }

    return false
}



export const reqMakeOrder = (baseData: any, quote: QuoteData, unfinishedSteps: number) => {
    const { side, offset, accountId, volume, parentId } = baseData;
    const { instrumentTypeOrigin, instrumentId, exchangeId } = quote;

    const makeOrderPrice = getMakeOrderPrice(side, quote, unfinishedSteps)

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

function getMakeOrderPrice (side: number, quote: QuoteData, unfinishedSteps: number ) {
    const { upperLimitPrice, lowerLimitPrice, lastPrice, askPrices, bidPrices } = quote;
    
    if (+side === 0) {
        if (unfinishedSteps > 1) {
            return bidPrices[0]
        } else {
            return upperLimitPrice
        }
    } else if (+side === 1) {
        if (unfinishedSteps > 1) {
            return askPrices[0]
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
}

function dealMakeOrderVolume (instrumentType: number, volume: number) {
    //stock 100的倍数
    const scale100 = Object.keys(InstrumentTypes)
        .filter(key => {
            if (key.toLowerCase().includes('stock')) return true;
            return false;
        })
        .map(key => +InstrumentTypes[key])
    
    if (scale100.includes(+instrumentType)) {
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
    const currentVolumeData: any = {
        [+TARGET_DIRECTION]: currentVolume,
        [+TARGET_DIRECTION_CONT]: currentVolumeCont
    }
    const totalTargetVolume: number = targetPosData
    .map((item: VolumeRecordData): number => {
        const { d, v } = item;
        const currentV = currentVolumeData[d]
        const delta = currentV - v
        //需要根据开平方向判断， 开->大于目标仓位->0，平->小于目标仓位->0
        if (d === TARGET_DIRECTION) {
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
    }) || 0

    const targetVolumeByStep = unfinishedSteps === 1 ? totalTargetVolume : totalTargetVolume / unfinishedSteps

    return {
        currentVolume,
        currentVolumeCont,
        total: totalTargetVolume,
        thisStepVolume: dealMakeOrderVolume( instrumentType, targetVolumeByStep)
    }
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
    loopInterval,
    LAST_SINGULARITY_SECOND,
    LAST_STEP_COUNT
}: {
    currentTimestamp: number; 
    deltaMilliSeconds: number;
    finishTime: number;
    loopInterval: number;
    LAST_SINGULARITY_SECOND: number;
    LAST_STEP_COUNT: number
}) => {
    const deltaCurrentToFinishTime = finishTime - currentTimestamp;
    const currentCount = Math.floor(deltaMilliSeconds / loopInterval)
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