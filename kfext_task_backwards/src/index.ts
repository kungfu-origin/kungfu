import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
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
} from './assets/utils';
import moment from 'moment';


const argv = minimist(process.argv.slice(2), {
    string: ['tickerSet', 'index', 'accountId'],
    boolean: []
})

const { tickerSet, tickerSetTickers, index, side, interval, maxBackward } = argv;

const triggerTime = moment().valueOf()
const finishTime = moment().endOf('day').valueOf()

const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const LOOP_INTERVAL = interval;
const TICKERS = tickerSetTickers.split('=');
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

const combineLatestObserver = combineLatest(
    TIMER_COUNT_OBSERVER(),
    quotesPipe(),
    instrumentsPipe(),
    avgVolumePipe(),
    (
        seconds: number,
        quotes: StringToQuoteData,
        instruments: StringToInstrumentData,
        avgVolume: StringToNumberObject,        
    ) => {
        
        return {
            seconds,
            quotes,
            instruments,
            avgVolume,
        }
    }
)


var dealedCount = -10000000000;
var checkRequiredDataErrorLogged = false;


combineLatestObserver
    .pipe(
        filter((data: BackWardTraderPipeData) => {
            const { avgVolume, quotes, instruments } = data;
            const tickersNoIndex = TICKERS.filter((ticker: string) => !ticker.includes(index))
            
            if (Object.keys(avgVolume).length === 0) {
                if (!checkRequiredDataErrorLogged) {
                    console.error(`[WARNING] 暂无历史均成交量信息信息，请检查`)
                    checkRequiredDataErrorLogged = true;
                }
                return false;
            } else if (!ensureTargetIncludesAllKeys(quotes, TICKERS)) {
                if (!checkRequiredDataErrorLogged) {
                    console.log(Object.keys(quotes), TICKERS)
                    console.error(`[WARNING] 暂无行情信息，需保证已订阅${TICKERS}, 且MD进程开启`)
                    checkRequiredDataErrorLogged = true;
                }
                return false;
            } else if (!ensureTargetIncludesAllKeys(instruments, tickersNoIndex)) {
                if (!checkRequiredDataErrorLogged) {
                    console.log(Object.keys(instruments), tickersNoIndex)
                    console.error(`[WARNING] 暂无到期日信息，需保证已订阅${tickersNoIndex}, 需保证MD进程开启`)
                    checkRequiredDataErrorLogged = true;
                }
                return false;

            } else if (!TICKERS.filter((ticker: string) => ticker.includes(index)).length) {
                if (!checkRequiredDataErrorLogged) {
                    console.error(`[WARNING] ${TICKERS} 不包含指數 ${index}`)
                    finishTrade('failed')
                    checkRequiredDataErrorLogged = true;
                }
                return false;
            }

            checkRequiredDataErrorLogged = false;
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

        if (!tickersResolved.length ) {
            console.log('[ERROR] 无合约')
            finishTrade('failed')
            return;
        }

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
                instrumentPrice: quoteData.lastPrice,
                indexId,
                indexP,
                backwardsDelta: +Number(lastPrice - indexP).toFixed(3),
                backWardsRatio: indexP * lastPrice === 0 ? '' : Number((lastPrice - indexP) / lastPrice).toFixed(3),
                backwardByYear: +side === 0 
                    ? ensureNum(+Number(Math.abs((ask1 - indexP) / ask1 / toExpireDate * 365)).toFixed(3))
                    : ensureNum(+Number(Math.abs((bid1 - indexP) / bid1 / toExpireDate * 365)).toFixed(3))
            }

        });

        //计算最大贴水
        const minBackWardData = getTickerWithMinValue(Object.values(combinedInstrumentData), 'backwardByYear')
        const minBackWard = minBackWardData.backwardByYear;

        console.log(`[计算结果] 最小贴水 ${minBackWardData.name} ${minBackWard}`)
        
        let selectedInstrument = '';

        console.log(+minBackWard, +MAX_BACKWARD, +minBackWard > +MAX_BACKWARD)
        
        if (+minBackWard > +MAX_BACKWARD ) {
            console.log(`[判断] 最小贴水 > 最大贴水 ${MAX_BACKWARD}, 找出流动性最大的合约`)

            //找出流动性最大的合约
            const maxLotData = getTickerWithMaxValue(Object.values(combinedInstrumentData), 'avg7Volume')
            console.log(`[计算结果] 流动性最大 ${maxLotData.name} ${maxLotData.avg7Volume}`);
            selectedInstrument = maxLotData.name

        } else {
            console.log(`[判断] 最小贴水 < 最大贴水 ${MAX_BACKWARD}, 在贴水小于_MaxBackward的合约中, 找出流动性最大的合约, 排除当月合约`)

            //在贴水小于_MaxBackward的合约中，找出流动性最大的合约，排除当月合约
            const lessThanMaxBackwardList = Object.values(combinedInstrumentData)
                .filter((item: any) => {
                    const { expireDate } = item;
                    return moment(expireDate).format('YYYYMM') !== moment().format('YYYYMM')
                })
                .filter((item: any) => {
                    return +item.backwardByYear <= +MAX_BACKWARD
                })

            if (!lessThanMaxBackwardList.length) {
                console.log('[计算结果] 排除当月合约后，无标的剩余')
            } else {
                const maxLotData = getTickerWithMaxValue(lessThanMaxBackwardList, 'avg7Volume')
                console.log(`[计算结果] 流动性最大 ${maxLotData.name} ${maxLotData.avg7Volume}`);
                selectedInstrument = maxLotData.name
            }
        }

        Object.keys(combinedInstrumentData || {})
            .forEach((key: string) => {
                const instrumentData: any = combinedInstrumentData[key];
                recordTaskInfo({
                    ...instrumentData,
                    selected: instrumentData.name === selectedInstrument
                }, argv)
                
            })
        
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

        process.exit(0)
    }

