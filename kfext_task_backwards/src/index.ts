import minimist from 'minimist';
import { Observable, combineLatest, timer } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import { 
    transformArrayToObjectByKey, 
    getCurrentTimestamp, 
    reqTimer,
    getTickerWithMinValue,
    getTickerWithMaxValue,
    ensureTargetIncludesAllKeys,
    ensureNum,
    recordTaskInfo,
    reqMakeOrder 
} from './assets/utils';
import moment from 'moment';


const argv = minimist(process.argv.slice(2), {
    string: ['tickerSet', 'index'],
    boolean: ['sim']
})

const { tickerSet, tickerSetTickers, index, side, offset, volume, triggerTime, finishTime, interval, accountId, maxBackward, parentId, sim } = argv;

const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const LOOP_INTERVAL = interval;
const TICKERS = tickerSetTickers.split('=');
const PARENT_ID = parentId; 
const MAX_BACKWARD = Number(maxBackward / 100).toFixed(3);

console.log('==================== 交易信息 =======================')
console.log('[ARGS]', process.argv.slice(2).join(','))
console.log('===================================================')
console.log('[开始时间]', triggerTime, triggerTimeStr)
console.log('[结束时间]', finishTime, finishTimeStr)
console.log('[执行间隔]', LOOP_INTERVAL, 'ms')
console.log('[目标标的池] ', tickerSet, '>>>', TICKERS.join(', '))
console.log('[对应指数]', index)
console.log('[最大贴水]', MAX_BACKWARD)
console.log('===================================================')



// @ts-ignore
process.send({
    type: 'process:msg',
    data: {
        type: "SUBSCRIBE_BY_TICKERSET",
        body: {
            tickerSet: tickerSet,
        }
    }
})
console.log(`[订阅] 目标标的池 ${tickerSet} 内所有标的 ${TICKERS.join(', ')}`)


//请求行情
var reqQuoteTimer = reqTimer(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: "REQ_QUOTE_DATA",
            body: {
                ticker: TICKERS.join(',')
            }
        }
    })
}, 300)

//请求历史均
var reqHistoryAvgVolumeTimer = reqTimer(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: "REQ_HIS_AVG_VOLUME",
            body: {
                days: 7
            }
        }
    })
}, 5000)


// var reqPosOrderDataTimer = reqTimer(() => {
//     //@ts-ignore
//     process.send({
//         type: 'process:msg',
//         data: {
//             type: 'REQ_POS_ORDER_DATA',
//             body: {
//                 parentId: PARENT_ID,
//                 accountId
//             }
//         }
//     })
// }, 500)


//时间tick
var secondsCounterTimer: any = null;
const TIMER_COUNT_OBSERVER = (): Observable<number> => new Observable((subscriber) => {
    secondsCounterTimer = setInterval(() => {
        const currentTimestamp = +getCurrentTimestamp()
        const deltaMilliSeconds = currentTimestamp - triggerTime;
        const currentSecond = +Number(deltaMilliSeconds / 1000).toFixed(0)
        subscriber.next(currentSecond)
    }, 50)
})

const PROCESS_MSG_OBSERVER = (): Observable<ProcPayload> => new Observable(subscriber => {
    process.on('message', (packet) => {
        const { type, topic, data } = packet;
        if (type !== 'process:msg') return;
        subscriber.next({
            topic,
            data
        })
    })
})

const quotesPipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'QUOTE_DATA'
        }),
        map((payload: ProcPayload): StringToQuoteData => {
            const { data } = payload;
            const { quotes } = data;
            return transformArrayToObjectByKey(quotes, ['instrumentId', 'exchangeId'])
        })
    )
}

const instrumentsPipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'INSTRUMENT_DATA'
        }),
        map((payload: ProcPayload): StringToInstrumentData => {
            const { data } = payload;
            const { instruments } = data;
            return transformArrayToObjectByKey(instruments, ['instrument_id', 'exchange_id'])
        })
    )
}

const avgVolumePipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'HIS_AVG_VOLUME'
        }),
        map((payload: ProcPayload): StringToNumberObject => {
            const { data } = payload;
            const { avgVolume } = data;
            return avgVolume
        })
    )
}

// const ordersPipe = () => {
//     return PROCESS_MSG_OBSERVER().pipe(
//         filter((payload: ProcPayload) => {
//             return payload.topic === 'ORDER_DATA'
//         }),
//         map((payload: ProcPayload): OrderData[] => {
//             const { data } = payload;
//             const { orders } = data;
//             return orders
//         })
//     )
// }

// const positionsPipe = () => {
//     return PROCESS_MSG_OBSERVER().pipe(
//         filter((payload: ProcPayload) => {
//             return payload.topic === 'POS_DATA'
//         }),
//         map((payload: ProcPayload): StringToPosData | null => {
//             const { data } = payload;
//             const { positions } = data;

//             if (!positions || !positions.length) {
//                 return null
//             }

//             const positionsAfterFilter = positions.filter(posData => {
//                 const { instrumentId } = posData;
//                 if (tickerSetTickers.includes(instrumentId.toString())) {
//                     return true;
//                 }
//                 return false;
//             })
//             return transformArrayToObjectByKey(positionsAfterFilter, ['instrumentId', 'directionOrigin'])
//         })
//     )
// }

const combineLatestObserver = combineLatest(
    TIMER_COUNT_OBSERVER(),
    quotesPipe(),
    instrumentsPipe(),
    avgVolumePipe(),
    // ordersPipe(),
    // positionsPipe(),
    (
        seconds: number,
        quotes: StringToQuoteData,
        instruments: StringToInstrumentData,
        avgVolume: StringToNumberObject,
        // orders: OrderData[],
        // pos: StringToPosData | null
        
    ) => {
        
        return {
            seconds,
            quotes,
            instruments,
            avgVolume,
            // orders,
            // pos
        }
    }
)


var tradeVolumeByStep = 1; //initLot;
var tradedVolume = 0; //flag
var dealedCount = -10000000000;


combineLatestObserver
    .pipe(
        filter(() => {
            if (+new Date().getTime() > finishTime) {
                finishTrade('time')
                return false;
            } 

            return true
        })
    )
    .pipe(
        filter((data: BackWardTraderPipeData) => {
            const { avgVolume, quotes, instruments } = data;
            const tickersNoIndex = TICKERS.filter((ticker: string) => !ticker.includes(index))
            
            if (Object.keys(avgVolume).length === 0) {
                console.error(`[WARNING] 暂无历史均成交量信息信息，请检查`)
                return false;
            } else if (!ensureTargetIncludesAllKeys(quotes, TICKERS)) {
                console.log(Object.keys(quotes), TICKERS)
                console.error(`[WARNING] 暂无行情信息，需保证已订阅${TICKERS}, 且MD进程开启`)
                return false;
            } else if (!ensureTargetIncludesAllKeys(instruments, tickersNoIndex)) {
                console.log(Object.keys(instruments), tickersNoIndex)
                console.error(`[WARNING] 暂无到期日信息，需保证已订阅${tickersNoIndex}, 需保证MD进程开启`)
                return false;

            } else if (!TICKERS.filter((ticker: string) => ticker.includes(index)).length) {
                console.error(`[WARNING] ${TICKERS} 不包含指數 ${index}`)
                finishTrade('failed')
                return false;
            }
            
            return true;
        })
    )
    .pipe(
        filter((data: BackWardTraderPipeData) => {
            const { seconds } = data;
            const count = +Math.ceil((+new Date().getTime() - triggerTime) / interval);
            
            if (count < 0) return false;
            if (dealedCount === count) return false;
            
            dealedCount = count;
            
            console.log('============================================================')
            console.log(`=============== [PROCESS] Loop Start at ${seconds}s ================`)
            console.log('============================================================')

            return true;
        })
    )
    .subscribe((data: BackWardTraderPipeData) => {

        const { quotes, instruments, avgVolume } = data;

        const indexId = TICKERS.filter((ticker: string) => ticker.includes(index))[0]
        const indexData = quotes[indexId] || {};
        const indexP = ensureNum(indexData.lastPrice || 0);
        const today = moment(moment().format('YYYYMMDD'))

        let combinedInstrumentData:any = {};

        const tickersResolved = TICKERS.filter((ticker: string) => !ticker.includes(index))
        tickersResolved.forEach((instrumentId_exchangeId: string) => {
            const quoteData = quotes[instrumentId_exchangeId] || {};
            const instrumentData = instruments[instrumentId_exchangeId] || {};
            const bid1: number = ensureNum(+(quoteData.bidPrices || [])[0] || 0);
            const ask1: number = ensureNum(+(quoteData.askPrices || [])[0] || 0);
            const lastPrice: number = ensureNum(quoteData.lastPrice);
            const expireDate = instrumentData.expire_date || '';
            const toExpireDate = moment(expireDate).diff(today, 'days'); 

            combinedInstrumentData[instrumentId_exchangeId] = {
                name: instrumentId_exchangeId,
                avg7Volume: avgVolume[instrumentId_exchangeId],
                toExpireDate, 
                expireDate,
                bid1,
                ask1,
                indexId,
                indexP,
                backwardsDelta: lastPrice - indexP,
                backWardsRatio: indexP * lastPrice === 0 ? '' : Number((lastPrice - indexP) / lastPrice).toFixed(3),

                backwardByYear: +side === 0 
                    ? ensureNum(+Number(Math.abs((ask1 - indexP) / ask1 / toExpireDate * 365)).toFixed(3))
                    : ensureNum(+Number(Math.abs((bid1 - indexP) / bid1 / toExpireDate * 365)).toFixed(3))
            }

            recordTaskInfo(combinedInstrumentData[instrumentId_exchangeId], {}, {
                ...argv,
                tradedVolume
            })
        });

        //计算最大贴水
        const minBackWardData = getTickerWithMinValue(Object.values(combinedInstrumentData), 'backwardByYear')
        const minBackWard = minBackWardData.value;

        console.log(`[计算结果] 最小贴水 ${minBackWardData.name} ${minBackWardData.backwardByYear}`)
        
        if (minBackWard > MAX_BACKWARD ) {
            console.log(`[判断] 最小贴水 > 最大贴水 ${MAX_BACKWARD}, 找出流动性最大的合约`)

            //找出流动性最大的合约
            const maxLotData = getTickerWithMaxValue(Object.values(combinedInstrumentData), 'avg7Volume')
            console.log(`[计算结果] 流动性最大 ${maxLotData.name} ${maxLotData.avg7Volume}`);
            const makeOrderData = reqMakeOrder({ ...argv, volume: tradeVolumeByStep}, quotes[maxLotData.name]);
            tradedVolume = tradedVolume + tradeVolumeByStep;
            
            if (makeOrderData) {
                recordTaskInfo(maxLotData, makeOrderData, {
                    ...argv,
                    tradedVolume
                })
            }

        } else {
            console.log(`[判断] 最小贴水 < 最大贴水 ${MAX_BACKWARD}, 在贴水小于_MaxBackward的合约中, 找出流动性最大的合约, 排除当月合约`)

            //在贴水小于_MaxBackward的合约中，找出流动性最大的合约，排除当月合约
            const lessThanMaxBackwardList = Object.values(combinedInstrumentData)
                .filter((item: any) => {
                    const { expireDate } = item;
                    return moment(expireDate).format('YYYYMM') !== moment().format('YYYYMM')
                })
                .filter((item: any) => {
                    return item.backwardByYear <= MAX_BACKWARD
                })

            if (!lessThanMaxBackwardList.length) {
                console.log('[计算结果] 排除当月合约后，无标的剩余')
            } else {
                const maxLotData = getTickerWithMaxValue(lessThanMaxBackwardList, 'avg7Volume')
                console.log(`[计算结果] 流动性最大 ${maxLotData.name} ${maxLotData.avg7Volume}`);
                const makeOrderData = reqMakeOrder({ ...argv, volume: tradeVolumeByStep}, quotes[maxLotData.name]);                
                tradedVolume = tradedVolume + tradeVolumeByStep
                
                if (makeOrderData) {
                    recordTaskInfo(maxLotData, makeOrderData, {
                        ...argv,
                        tradedVolume
                    })
                }
            }
        }

        const timeRemain = +Number(( +new Date().getTime() - finishTime) / ( triggerTime - finishTime )).toFixed(3);
        const tradeRemain = 1 - tradedVolume / volume;

        if (tradeRemain - timeRemain > 0.2) {
            console.log(`[WARNING] tradeRemain ${tradeRemain}`)
            if (tradeVolumeByStep < 3) { 
                tradeVolumeByStep = tradeVolumeByStep + 1
            }
        }

        if (tradeRemain - timeRemain > 0.5) {
            console.log(`[WARNING] tradeRemain ${tradeRemain}`)
            if (tradeVolumeByStep < 5) { 
                tradeVolumeByStep = tradeVolumeByStep + 2
            }
        }

        //剩余不够，则算差值
        if (tradeVolumeByStep + tradedVolume > volume) {
            tradeVolumeByStep = volume - tradedVolume
        }

        if (tradedVolume >= volume) {
            finishTrade('trade')
        }

        if (+new Date().getTime() > finishTime) {
            finishTrade('time')
        }
    })

    function finishTrade (type: string) {
        if (type === 'time') {
            console.log(`====================== 时间截止，交易结束 ======================`)
        }
        
        if (type === 'trade') {
            console.log(`====================== 目标完成，交易结束 ======================`)
        }

        if (type === 'failed') {
            console.log(`================== 失败，交易条件未满足，交易结束 ===================`)
        }

        reqQuoteTimer && clearInterval(reqQuoteTimer)
        secondsCounterTimer && clearInterval(secondsCounterTimer)
        reqHistoryAvgVolumeTimer && clearInterval(reqHistoryAvgVolumeTimer)
        // reqPosOrderDataTimer && clearInterval(reqPosOrderDataTimer)

        process.exit(0)
    }

