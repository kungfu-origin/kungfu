import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import moment from 'moment';
import { 
    transformArrayToObjectByKey, 
    makeOrderDirectionType, 
    buildTarget, 
    reqMakeOrder, 
    getAliveOrders,
    reqCancelOrder,
    calcVolumeThisStep,
    timeCheckBySecond,
    getCurrentCount,
    buildTradeTaskVolumeOffset,
    getCurrentTimestamp,
    getCancelOrderBeforeLastStepControllerTime,
    printQuote,
    recordTaskInfo
} from './assets/utils';


const argv = minimist(process.argv.slice(2), {
    string: 'ticker',
    boolean: 'lastSingularity',
})
const { ticker, side, offset, volume, steps, triggerTime, finishTime, exchangeId, parentId, accountId, lastSingularity, lastSingularityMilliSecond, maxLotByStep } = argv;
const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const LOOP_INTERVAL = Math.ceil((finishTime - triggerTime) / steps);
const TICKER = ticker.toString().trim();
const PARENT_ID = parentId; 
    
const TARGET_DIRECTION = makeOrderDirectionType(side, offset).d;
const TARGET_DIRECTION_CONT = makeOrderDirectionType(side, offset).dc;
const OPERATION_NAME = makeOrderDirectionType(side, offset).n;
const TARGET_VOLUME = volume;
const LAST_STEP_COUNT = steps - 1;
const LAST_SINGULARITY_SECOND = lastSingularity ? lastSingularityMilliSecond : 0

console.log('==================== 交易信息 =======================')
console.log('[ARGS]', process.argv.slice(2).join(','))
console.log('===================================================')
console.log('[开始时间]', triggerTime, triggerTimeStr)
console.log('[结束时间]', finishTime, finishTimeStr)
console.log('[执行间隔]', LOOP_INTERVAL, 'ms')
console.log('[目标标的] ', TICKER)
console.log('===================================================')


//@ts-ignore
process.send({
    type: 'process:msg',
    data: {
        type: 'SUBSCRIBE_BY_TICKER',
        body: {
            ticker: TICKER,
            exchangeId,
            accountId
        }
    }
})

console.log(`[订阅] ${TICKER} ${exchangeId} ${accountId}`)

//ledger data request
var reqTradingDataTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_LEDGER_DATA',
            body: {
                parentId: PARENT_ID,
                accountId,
                ticker: TICKER
            }
        }
    })
}, 100)

var secondsCounterTimer: any = null;
const TIMER_COUNT_OBSERVER = (): Observable<TimeCountData> => new Observable((subscriber) => {
    secondsCounterTimer = setInterval(() => {
        const currentTimestamp = +getCurrentTimestamp()
        const deltaMilliSeconds = currentTimestamp - triggerTime;
        const currentCount = getCurrentCount({
            currentTimestamp, 
            deltaMilliSeconds,
            finishTime,
            LOOP_INTERVAL,
            LAST_SINGULARITY_SECOND,
            LAST_STEP_COUNT
        })
        const currentSecond = +Number(deltaMilliSeconds / 1000).toFixed(0)

        subscriber.next({
            count: currentCount, 
            second: currentSecond
        })
    
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
            return payload.topic === 'LEDGER_DATA'
        }),
        map((payload: ProcPayload): StringToQuoteData => {
            const { data } = payload;
            const { quotes } = data;
            const quotesAfterFilter = quotes.filter(quoteData => {
                const instrumentId = quoteData['instrumentId'];
                return TICKER === instrumentId.toString()
            })
            return transformArrayToObjectByKey(quotesAfterFilter, ['instrumentId'])
        })
    )
}

const ordersPipe = () => {
    return PROCESS_MSG_OBSERVER().pipe(
        filter((payload: ProcPayload) => {
            return payload.topic === 'LEDGER_DATA'
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
            return payload.topic === 'LEDGER_DATA'
        }),
        map((payload: ProcPayload): null | { [prop: string]: PosData } => {
            const { data } = payload;
            const { positions } = data;

            if (!positions || !positions.length) {
                return null
            }

            const positionsAfterFilter = positions.filter(posData => {
                const { instrumentId } = posData;
                if (TICKER === instrumentId.toString()) {
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
    positionsPipe(),
    ordersPipe(),
    (
        timeCountData: TimeCountData, 
        quotes: StringToQuoteData, 
        positions: StringToPosData | null, 
        orders: OrderData[]
    ) => {
        const timeTraderPipData: TimeTraderPipData = {
            timeCountData,
            quotes,
            positions,
            orders
        }

        return timeTraderPipData
    }
    
)

var dealedTimeCount: number = -1000000000000;
var dealedSecond: number | undefined = undefined;
var lastestMakeOrdertimeStamp = 0;
var targetPosData: any = null;
var hasConsoledTotalFinished = false;
var hasCancelBeforeLastStep = false;

combineLatestObserver
    .pipe(
        //============================ 时间检查 start ============================
        filter((data: TimeTraderPipData)  => {
            const { timeCountData, quotes } = data
            //记录时间发送alert
            const quote = quotes[TICKER];
            const timeCount = timeCountData.count;
            const timeSecond = timeCountData.second;
            const currentTimestamp = getCurrentTimestamp()

            if (timeSecond !== dealedSecond && timeSecond < 0) {
                dealedSecond = timeSecond;
                if (timeSecond < 0) {
                    timeCheckBySecond(dealedSecond, quote)
                }
            }

            //时间到
            if ((timeCount > LAST_STEP_COUNT) || (currentTimestamp > finishTime)) {
                handleFinished(quote, printQuote)
                return false;
            }

            //时间未到
            if (timeCount < 0) {
                return false
            }
            return true
        })
    )
    .pipe(
        //===================== loop之间检查，如不通过则不会进入loop start ===========
        filter((data: TimeTraderPipData) => {
            const { quotes, positions } = data
            const quote = quotes[TICKER];

            if (!quote) {
                console.error(`[WARNING] 暂无${ticker}行情信息，需保证MD进程开启`)
                return false;
            }

            //制定全部交易计划
            const pos = (positions || {})[`${TICKER}_${TARGET_DIRECTION}`] || {};
            if (!targetPosData) {
                const { totalVolume } = pos;
                targetPosData = buildTarget({ 
                    offset,
                    side,
                    ticker,
                    totalVolume: totalVolume || 0,
                    targetVolume: TARGET_VOLUME
                })

                //依然没有
                if (!targetPosData) {
                    return false
                };
            }
            return true;
        })
    )
    .pipe(
        filter((data: TimeTraderPipData) => {
            const { timeCountData} = data
            const timeCount = timeCountData.count;
            if (timeCount <= dealedTimeCount) {

                //最后一步之前撤单
                if (dealedTimeCount === LAST_STEP_COUNT - 1) {
                    const lastStepInterval = Math.min(LAST_SINGULARITY_SECOND, LOOP_INTERVAL);
                    const cancelOrderBeforeLastStepTime =  getCancelOrderBeforeLastStepControllerTime(finishTime, lastStepInterval, lastestMakeOrdertimeStamp);
                    if (finishTime - +getCurrentTimestamp() <= cancelOrderBeforeLastStepTime) {
                        if (!hasCancelBeforeLastStep) {
                            hasCancelBeforeLastStep = true;
                            console.log(`[最后一步之前撤单] 距结束 ${getCurrentTimestamp(true)}`)
                            reqCancelOrder(PARENT_ID)
                        }
                    }
                }
                
                return false;
            }
            return true;
        })
    )
    .pipe(
        //===================== 进入loop检查，通过后才开始交易 start ================
        filter((data: TimeTraderPipData) => {
            const { timeCountData, orders } = data
            const timeCount = timeCountData.count;
            console.log(`[交易检查] ${timeCount + 1} / ${steps}, ${getCurrentTimestamp(true)} `)
            
            //判断是否可以交易, 如不能交易，先撤单
            const aliveOrders = getAliveOrders(orders)
            if (aliveOrders.length) {
                console.log(`[检查订单] 活动订单数量 ${aliveOrders.length} / ${orders.length}, 等待全部订单结束`)
                reqCancelOrder(PARENT_ID)

                //如果离最后截止时间小于50ms，也全部执行
                if ( finishTime - +getCurrentTimestamp() <= 50) {
                    console.log('[<=50ms] 最后一步强制执行')
                    return true
                }

                return false
            } 
            
            return true;
        })
    )
    .subscribe((data: TimeTraderPipData) => {
        const { timeCountData, quotes } = data;
        const timeCount = timeCountData.count;
        const quote = quotes[TICKER];
        const positions = data.positions || {}

        //============================= 交易环节 start =============================
        //制定本次交易计划
        const instrumentType = quote.instrumentTypeOrigin;
        const unfinishedSteps = resolveUnfinishedSteps(steps - timeCount);
        const { total, thisStepVolume, currentVolume, currentYesVolume, currentTodayVolume, currentVolumeCont }  = calcVolumeThisStep(
            positions,
            TICKER,
            TARGET_DIRECTION,
            TARGET_DIRECTION_CONT,
            offset,
            targetPosData,
            unfinishedSteps,
            instrumentType
        )

        if (+total === 0) {
            if (!hasConsoledTotalFinished) {
                console.log('================================================================')
                console.log(`====================== 交易任务完成 ==============================`)
                console.log('================================================================')
                hasConsoledTotalFinished = true;
            }
        
            return false;
        }

        console.log(`========== 交易条件满足，开始 ${timeCount + 1} / ${steps} =========`)

        const tradeTargetList: Array<TradeTarget> = buildTradeTaskVolumeOffset({
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
        })

        tradeTargetList.forEach((target: TradeTarget) => {
            const { offset, volume } = target;
            if (+offset === 2) {
                console.log(`[Extra] 平今 ${volume}`)
            } else if (+offset === 3) {
                console.log(`[Extra] 平昨 ${volume}`)
            }

            if (+maxLotByStep > 0) {
                splitMakeOrderStep(maxLotByStep, volume, (stepVolume: number) => {
                    const makeOrderData = reqMakeOrder({ ...argv, offset, volume: stepVolume }, quote, unfinishedSteps)  
                    makeOrderData && recordTaskInfo(quote, makeOrderData, {
                        ...argv,
                        volumeLefted: total,
                    })
                })
            } else {
                const makeOrderData = reqMakeOrder({ ...argv, offset, volume }, quote, unfinishedSteps);
                makeOrderData && recordTaskInfo({...quote}, makeOrderData, {
                    ...argv,
                    volumeLefted: total,
                })
            }

            console.log(`[下单时间] ${getCurrentTimestamp(true)}`)  
        })

        console.log(`============ 已完成执行 ${timeCount + 1} / ${steps} ==============`)    
        dealedTimeCount = timeCount; //此时记录下来
        lastestMakeOrdertimeStamp = +getCurrentTimestamp();
        //============================= 交易环节 end =============================
    })

function resolveUnfinishedSteps (unfinishiedSteps: number) {
    //有时虽然不是最后一步，但是时间马上截止
    const currentTimestamp: number = +getCurrentTimestamp();
    const limitValue = lastSingularity ? lastSingularityMilliSecond : LOOP_INTERVAL;
    if (finishTime - currentTimestamp <= limitValue) {
        console.log(`[时间截止前，最后一步] ${getCurrentTimestamp(true)}`)
        return 1
    } 
    return unfinishiedSteps
}

function handleFinished (quote: QuoteData, printQuote: Function) {
    console.log(`====================== 时间截止，交易结束 ======================`)
    console.log('[收盘]')
    printQuote(quote)
    secondsCounterTimer && clearInterval(secondsCounterTimer)
    reqTradingDataTimer && clearInterval(reqTradingDataTimer)
    process.exit(0)
}

function splitMakeOrderStep (limit: number, total: number, cb: Function ) {
    let left = total;
    while (left > limit) {
        left = left - limit
        cb && cb(limit)
    }
    cb && cb(left)
}

