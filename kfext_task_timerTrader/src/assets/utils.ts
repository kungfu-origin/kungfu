import moment from 'moment';

export const transformArrayToObjectByKey = (targetList: Array<any>, keys: Array<string>): any => {
    let data: any = {};
    (targetList || []).forEach(item => {
        const key:string = keys.map(k => item[k]).join('_')
        data[key] = item
    })

    return data
}

export const makeOrderDirectionType = (side: number, offset: number): number => {
    if (+side === 0) {
        if (+offset === 0) {
            return 0 // 'long-open'
        } else if (+offset === 1) {
            return 1 // 'short-close'
        }
    } else if (+side === 1) {
        if (+offset === 0) {
            return 1 // 'short-open'
        } else if (+offset === 1) {
            return 0 // 'long-close'
        }
    }

    return 0
}

export const buildTarget = ({ offset, side, ticker, totalVolume, targetVolume }: { 
    offset: number,
    side: number,
    ticker: string,
    totalVolume: number,
    targetVolume: number
}) => {
    if (+side === 0) {
        if (+offset === 0) {
            console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标买开 ${targetVolume}`)    
            return [
                { [ `${ticker}_0` ]: totalVolume + targetVolume }
            ]            
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TRAGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}，现有持仓不足，需买平 ${totalVolume}，买开 ${delta}`)                
                return [
                    { [ `${ticker}_1` ]: 0 },
                    { [ `${ticker}_0` ]: delta }
                ]
            } else {
                console.log(`[TRAGET] 标的 ${ticker}，现有空仓 ${totalVolume}，目标买平 ${targetVolume}`)    
                return [
                    { [ `${ticker}_1` ]: totalVolume - targetVolume },
                ]
            }
        }
    } else if (+side === 1) {
        if (+offset === 0) {
            console.log(`[TRAGET] 标的 ${ticker}，现有空仓${totalVolume}，目标卖开${targetVolume}`)      
            return [
                { [ `${ticker}_1` ]: totalVolume + targetVolume },
            ]
        } else if (+offset === 1) {
            if (targetVolume > totalVolume) {
                const delta = targetVolume - totalVolume;
                console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}，现有持仓不足，需卖平 ${totalVolume}，卖开 ${delta}`)                
                return [
                    { [ `${ticker}_0` ]: 0 },
                    { [ `${ticker}_1` ]: delta }
                ]   
            } else {
                console.log(`[TRAGET] 标的 ${ticker}，现有多仓 ${totalVolume}，目标卖平 ${targetVolume}`)                
                return [
                    { [ `${ticker}_0` ]: totalVolume - targetVolume },
                ]
            }
        }
    }

    return undefined
}



export const reqMakeOrder = (baseData: any, quote: QuoteData, pos: PosData) => {
    const { ticker, side, offset, steps, accountId, targetVolume, timeCount, parentId } = baseData;

    if (!quote) {
        console.error(`暂无${ticker}行情信息`)
        return
    }

    if (!pos) {
        console.error(`暂无${ticker}_${targetVolume}持仓信息`)
        return
    }

    const { lastPrice, upperLimitPrice, lowerLimitPrice, instrumentId, exchangeId } = quote;
    const { instrumentType } = pos;

    //先撤单

    //再下单
    const unfinishedSteps = steps - timeCount || 1;
    if (unfinishedSteps < 0) {
        console.error('[ERROR] steps - timeCount = ', unfinishedSteps)
    }
    const targetVolumeThisStep = Math.ceil(targetVolume / unfinishedSteps);
    const theVolume = dealMakeOrderVolume(instrumentType, targetVolumeThisStep)
    const makeOrderData = { 
        name: accountId,
        instrument_id: instrumentId,
        instrument_type: instrumentType,
        exchange_id: exchangeId,
        limit_price: lastPrice,
        volume: theVolume || 0,
        side: side,
        offset: offset,
        price_type: 0,
        hedge_flag: 0
    }
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'MAKE_ORDER_BY_PARENT_ID',
            parentId: parentId,
            body: {
                ...makeOrderData
            }
        }
    })
    console.log(`[下单] ${JSON.stringify(makeOrderData)}`)
}

function dealMakeOrderVolume (instrumentType: number, volume: number) {
    const scale100 = [1] //stock 100的倍数
    
    if (scale100.includes(+instrumentType)) {
        const scale = +Number(volume / 100).toFixed(0)
        const scaleResolved = scale < 1 ? 1 : scale;
        return scaleResolved * 100
    }

    return volume
}