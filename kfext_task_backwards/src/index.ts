import minimist from 'minimist';
import { Observable, combineLatest, timer } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import { 
    transformArrayToObjectByKey, 
    getCurrentTimestamp, 
    reqTimer,
    getTickerWithMinValue,
    getTickerWithMaxValue,
    reqMakeOrder 
} from './assets/utils';
import moment from 'moment';


const argv = minimist(process.argv.slice(2), {
    string: ['tickerSet', 'index'],
    boolean: ['sim']
})

const { tickerSet, tickerSetTickers, index, side, offset, volume, triggerTime, finishTime, interval, accountId, maxBackward, parentId } = argv;

const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const LOOP_INTERVAL = interval;
const TICKERS = tickerSetTickers.split('=');
const PARENT_ID = parentId; 

console.log('==================== 交易信息 =======================')
console.log('[ARGS]', process.argv.slice(2).join(','))
console.log('===================================================')
console.log('[开始时间]', triggerTime, triggerTimeStr)
console.log('[结束时间]', finishTime, finishTimeStr)
console.log('[执行间隔]', LOOP_INTERVAL, 'ms')
console.log('[目标标的池] ', tickerSet, '>>>', TICKERS.join(', '))
console.log('[对应指数]', index)
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

var reqPosOrderDataTimer = reqTimer(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_POS_ORDER_DATA',
            body: {
                parentId: PARENT_ID,
                accountId
            }
        }
    })
}, 500)


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
        console.log(topic, '--')
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

const ordersPipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'POS_ORDER_DATA'
        }),
        map((payload: ProcPayload): OrderData[] => {
            const { data } = payload;
            const { orders } = data;
            return orders
        })
    )
}

const positionsPipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'POS_ORDER_DATA'
        }),
        map((payload: ProcPayload): StringToPosData | null => {
            const { data } = payload;
            const { positions } = data;

            if (!positions || !positions.length) {
                return null
            }

            const positionsAfterFilter = positions.filter(posData => {
                const { instrumentId } = posData;
                if (tickerSetTickers.includes(instrumentId.toString())) {
                    return true;
                }
                return false;
            })
            return transformArrayToObjectByKey(positionsAfterFilter, ['instrumentId', 'directionOrigin'])
        })
    )
}

const combineLatestObserver = combineLatest(
    TIMER_COUNT_OBSERVER(),
    quotesPipe(),
    instrumentsPipe(),
    avgVolumePipe(),
    ordersPipe(),
    positionsPipe(),
    (
        seconds: number,
        quotes: StringToQuoteData,
        instruments: StringToInstrumentData,
        avgVolume: StringToNumberObject,
        orders: OrderData[],
        pos: StringToPosData | null
        
    ) => {
        
        return {
            seconds,
            quotes,
            instruments,
            avgVolume,
            orders,
            pos
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
            const { avgVolume, quotes } = data;
            if (Object.keys(avgVolume).length === 0) {
                console.error(`[WARNING] 暂无历史均成交量信息信息，请检查`)
                return false;
            } else if (!quotes) {
                console.error(`[WARNING] 暂无行情信息，需保证MD进程开启`)
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
            console.log(`=============== [PROCESS] Loop Start at ${seconds}s ===============`)
            console.log('============================================================')

            return true;
        })
    )
    .subscribe((data: BackWardTraderPipeData) => {

        const { quotes, instruments, avgVolume } = data;
        const indexFullId = TICKERS.filter((ticker: string) => ticker.includes(index))[0]
        const indexData = quotes[indexFullId] || {};
        const indexP = +indexData.lastPrice || 0;
    

        console.log(instruments, '---')
        
        //combine data
        let combinedInstrumentData:any = {};
        const today = moment(moment().format('YYYYMMDD'))

        TICKERS.forEach((instrumentId_exchangeId: string) => {
            const quoteData = quotes[instrumentId_exchangeId] || {};
            const instrumentData = instruments[instrumentId_exchangeId] || {};
            const bid1: number = +(quoteData.bidPrices || [])[0] || 0;
            const ask1: number = +(quoteData.askPrices || [])[0] || 0;
            const expireDate = instrumentData.expire_date || '';
            const toExpireDate = moment(expireDate).diff(today, 'days'); 

            combinedInstrumentData[instrumentId_exchangeId] = {
                avg7Volume: avgVolume[instrumentId_exchangeId],
                toExpireDate, 
                expireDate: 
                bid1,
                ask1,
                backwardByYear: +side === 0 
                    ? +Number(Math.abs((ask1 - indexP) / ask1 / toExpireDate * 365)).toFixed(3)
                    : +Number(Math.abs((bid1 - indexP) / bid1 / toExpireDate * 365)).toFixed(3)
            }

            console.log(JSON.stringify(combinedInstrumentData[instrumentId_exchangeId]))
        });
        

        return 

        // //price: P, price data: PD
        // const indexPD = quotes[index] || {}
        // const f0PD =    quotes[T_M_TICKER] || {};
        // const f1PD =    quotes[N_M_TICKER] || {};
        // const f2PD =    quotes[T_S_TICKER] || {};
        // const f3PD =    quotes[N_S_TICKER] || {};
        
        // const indexP =  +indexPD.lastPrice || 0
        // const f0BP =    +(f0PD.bidPrices || [])[0] || 0    
        // const f1BP =    +(f1PD.bidPrices || [])[0] || 0    
        // const f2BP =    +(f2PD.bidPrices || [])[0] || 0    
        // const f3BP =    +(f3PD.bidPrices || [])[0] || 0   
        
       
        // const f0AP =    +(f0PD.askPrices || [])[0] || 0    
        // const f1AP =    +(f1PD.askPrices || [])[0] || 0    
        // const f2AP =    +(f2PD.askPrices || [])[0] || 0    
        // const f3AP =    +(f3PD.askPrices || [])[0] || 0 

        // console.log(`[指数${index}点位] ${indexP}`)
        // console.log(`[当月合约${T_M_TICKER}价格] 买一${f0BP} 卖一${f0AP}`)
        // console.log(`[下月合约${N_M_TICKER}价格] 买一${f1BP} 卖一${f1AP}`)
        // console.log(`[当季合约${T_S_TICKER}价格] 买一${f2BP} 卖一${f2AP}`)
        // console.log(`[下季合约${N_S_TICKER}价格] 买一${f3BP} 卖一${f3AP}`)
        // console.log('----------------------------------')

        // //time: T
        // const today = moment(moment().format('YYYYMMDD'))
        // const f0T = moment((instruments[T_M_TICKER] || {}).expire_date || '').diff(today, 'days')
        // const f1T = moment((instruments[N_M_TICKER] || {}).expire_date || '').diff(today, 'days')
        // const f2T = moment((instruments[T_S_TICKER] || {}).expire_date || '').diff(today, 'days')
        // const f3T = moment((instruments[N_S_TICKER] || {}).expire_date || '').diff(today, 'days')  

        // console.log(`[当月合约${T_M_TICKER}剩余天数] ${f0T}`)
        // console.log(`[下月合约${N_M_TICKER}剩余天数] ${f1T}`)
        // console.log(`[当季合约${T_S_TICKER}剩余天数] ${f2T}`)
        // console.log(`[下季合约${N_S_TICKER}剩余天数] ${f3T}`)
        // console.log('----------------------------------')

        // //lot: L
        // const f0L = avgV0
        // const f1L = avgV1
        // const f2L = avgV2
        // const f3L = avgV3

        // console.log(`[当月合约${T_M_TICKER}周均成交量] ${f0L}`)
        // console.log(`[下月合约${N_M_TICKER}周均成交量] ${f1L}`)
        // console.log(`[当季合约${T_S_TICKER}周均成交量] ${f2L}`)
        // console.log(`[下季合约${N_S_TICKER}周均成交量] ${f3L}`)
        // console.log('----------------------------------')

        // //backward: B
        // var f0B, f1B, f2B, f3B;
        // if (side === 0) { //买
        //     f0B = +f0T === 0 ? 0 : +Number(Math.abs((f0AP - indexP) / f0AP / f0T * 365)).toFixed(3)
        //     f1B =                  +Number(Math.abs((f1AP - indexP) / f1AP / f1T * 365)).toFixed(3)
        //     f2B =                  +Number(Math.abs((f2AP - indexP) / f2AP / f2T * 365)).toFixed(3)
        //     f3B =                  +Number(Math.abs((f3AP - indexP) / f3AP / f3T * 365)).toFixed(3)
        // } else { //卖
        //     f0B = +f0T === 0 ? 0 : +Number(Math.abs((f0BP - indexP) / f0BP / f0T * 365)).toFixed(3)
        //     f1B =                  +Number(Math.abs((f1BP - indexP) / f1BP / f1T * 365)).toFixed(3)
        //     f2B =                  +Number(Math.abs((f2BP - indexP) / f2BP / f2T * 365)).toFixed(3)
        //     f3B =                  +Number(Math.abs((f3BP - indexP) / f3BP / f3T * 365)).toFixed(3)
        // }
       

        // console.log(`[当月合约${T_M_TICKER}年化贴水] ${f0B}`)
        // console.log(`[下月合约${N_M_TICKER}年化贴水] ${f1B}`)
        // console.log(`[当季合约${T_S_TICKER}年化贴水] ${f2B}`)
        // console.log(`[下季合约${N_S_TICKER}年化贴水] ${f3B}`)

        // console.log('========================================')

        // const backwardTickerDict: StringToNumberObject = {
        //     [T_M_TICKER]: f0B,
        //     [N_M_TICKER]: f1B,
        //     [T_S_TICKER]: f2B,
        //     [N_S_TICKER]: f3B,
        // }

        // const lotTickerDict: StringToNumberObject = {
        //     [T_M_TICKER]: f0L,
        //     [N_M_TICKER]: f1L,
        //     [T_S_TICKER]: f2L,
        //     [N_S_TICKER]: f3L,
        // }

        // const minBackWardData = getTickerWithMinValue(backwardTickerDict)
        // const minBackWard = minBackWardData.value;
        // const minBackwardName = minBackWardData.name;
        // console.log(`[最小年华贴水] ${minBackwardName} ${minBackWard}`);
        // console.log(`[最大年华贴水] ${maxBackward}`);

        // if (minBackWard > maxBackward) {
        //     const maxLotData = getTickerWithMaxValue(lotTickerDict)
        //     const targetTicker = maxLotData.name;
        //     console.log(`[最小贴水大于max贴水，最大流动性合约] ==== ${targetTicker} ${maxLotData.value}`);
        //     reqMakeOrder({ ...argv, volume: tradeVolumeByStep}, quotes[targetTicker]);
        //     tradedVolume = tradedVolume + tradeVolumeByStep
        // } else {
        //     let filteredLotTickerDict: StringToNumberObject = {};
        //     Object.keys(backwardTickerDict || {}).forEach(key => {
        //         const v = backwardTickerDict[key];
        //         if (v < maxBackward) {
        //             filteredLotTickerDict[key] = lotTickerDict[key]
        //         }
        //     })
        //     const maxLotData = getTickerWithMaxValue(filteredLotTickerDict)
        //     const targetTicker = maxLotData.name;
        //     console.log(`[小于max贴水中，最大流动性合约] ==== ${targetTicker} ${maxLotData.value}`);
        //     reqMakeOrder({ ...argv, volume: tradeVolumeByStep}, quotes[targetTicker]);
        //     tradedVolume = tradedVolume + tradeVolumeByStep
        // }

        // const timeRemain = +Number(( +new Date().getTime() - finishTime) / ( triggerTime - finishTime )).toFixed(3);
        // const tradeRemain = 1 - tradedVolume / volume;

        // if (tradeRemain - timeRemain > 0.2) {
        //     console.log(`[WARNING] tradeRemain ${tradeRemain}`)
        //     if (tradeVolumeByStep < 3) { 
        //         tradeVolumeByStep = tradeVolumeByStep + 1
        //     }
        // }

        // if (tradeRemain - timeRemain > 0.5) {
        //     console.log(`[WARNING] tradeRemain ${tradeRemain}`)
        //     if (tradeVolumeByStep < 5) { 
        //         tradeVolumeByStep = tradeVolumeByStep + 2
        //     }
        // }

        // if (tradedVolume >= volume) {
        //     finishTrade('trade')
        // }

        // if (+new Date().getTime() > finishTime) {
        //     finishTrade('time')
        // }
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
        reqPosOrderDataTimer && clearInterval(reqPosOrderDataTimer)

        process.exit(0)
    }

