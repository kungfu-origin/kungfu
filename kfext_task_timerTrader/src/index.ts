import minimist from 'minimist';
import { Observable, combineLatest, TimeInterval } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import moment from 'moment';
import { 
    transformArrayToObjectByKey, 
    makeOrderDirectionType, 
    buildTarget, 
    reqMakeOrder, 
    getAliveOrders,
    reqCancelOrder,
    calcVolumeThisStep
} from './assets/utils';




const argv = minimist(process.argv.slice(2), {
    string: 'ticker',
})
const { ticker, side, offset, volume, steps, triggerTime, finishTime, exchangeId, parentId, accountId } = argv;
const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const deltaTimestamp = Math.ceil((finishTime - triggerTime) / steps);
const TICKER = ticker.toString().trim();
const PARENT_ID = parentId; 
    
const TARGET_DIRECTION = makeOrderDirectionType(side, offset).d;
const OPERATION_NAME = makeOrderDirectionType(side, offset).n;
const TARGET_VOLUME = volume;
const LAST_STEP_COUNT = steps - 1;

console.log('==================== 交易信息 =======================')
console.log('[ARGS]', process.argv.slice(2).join(','))
console.log('===================================================')
console.log('[开始时间]', triggerTime, triggerTimeStr)
console.log('[结束时间]', finishTime, finishTimeStr)
console.log('[执行间隔]', deltaTimestamp, 'ms')
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

//行情request
var reqTradingDataTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_LEDGER_DATA',
            body: {
                parentId: PARENT_ID,
                accountId
            }
        }
    })
}, 1000)

var secondsCounterTimer: any = null;
const TIMER_COUNT_OBSERVER = (): Observable<number> => new Observable((subscriber) => {
    let count: number = -1;
    secondsCounterTimer = setInterval(() => {
        const currentTimestamp = moment().valueOf();
        
        if (currentTimestamp >= finishTime) {
            handleFinished()
            return;
        }

        const currentCount = Math.ceil((currentTimestamp - triggerTime) / deltaTimestamp)
        if (currentCount >= 0 && currentCount <= LAST_STEP_COUNT) {
            if (count !== currentCount) {
                subscriber.next(currentCount)
                count = +currentCount
            }
        }
    }, 500)
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
        map((payload: ProcPayload): { [prop: string]: QuoteData } => {
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
)

var dealedTimeCount: number = -1;
var targetPosData: any = null;
var hasCancelOrderInThisLoop = false;

combineLatestObserver.subscribe((
    [ timeCount, quotes, positions, orders ]: 
    [ number, { [prop: string]: QuoteData }, null | { [prop: string]: PosData }, OrderData[] ] 
) => {


    if (positions === null) {
        console.error(`[WARNING] 暂无${ticker}持仓信息`)
        return
    }

    //制定全部交易计划
    const pos = positions[`${TICKER}_${TARGET_DIRECTION}`];
    if (!targetPosData) {
        const { totalVolume } = pos || {};
        targetPosData = buildTarget({ 
            offset,
            side,
            ticker,
            totalVolume: totalVolume || 0,
            targetVolume: TARGET_VOLUME
        })

        //依然没有
        if (!targetPosData) {
            return
        };
    }

    const quote = quotes[TICKER];
    if (!quote) {
        console.error(`[WARNING] 暂无${ticker}行情信息`)
        return;
    }

    //必须在这里，以下都是在这个loop开始后执行
    if (timeCount <= dealedTimeCount) {
        return;
    }

    // 判断是否可以交易, 如不能交易，先撤单
    const aliveOrders = getAliveOrders(orders)
    if (aliveOrders.length) {
        if (!hasCancelOrderInThisLoop) {
            reqCancelOrder(PARENT_ID)
            hasCancelOrderInThisLoop = true
            console.log(`[检查订单] 活动订单数量 ${aliveOrders.length} / ${orders.length}, 等待全部订单结束`)
            console.log(`[撤单] PARENTID: ${PARENT_ID}`)
        }
        return
    } 
    
    //制定本次交易计划
    const instrumentType = quote.instrumentTypeOrigin;
    const unfinishedSteps = steps - timeCount
    const { total, thisStepVolume, currentVolume, currentVolumeCount }  = calcVolumeThisStep(
        positions,
        TICKER,
        TARGET_DIRECTION,
        targetPosData,
        unfinishedSteps,
        instrumentType
    )

    if (total === 0) {
        console.log('=========================================')
        console.log('=============== 交易任务完成 ==============')
        console.log('==========================================')
        handleFinished()
    }

    //时间到
    if (timeCount > LAST_STEP_COUNT) {
        handleFinished()
        return;
    }

    console.log(`========= 交易条件满足，开始 ${timeCount + 1} / ${steps} =========`)

    if ((offset === 0) || (currentVolume >= thisStepVolume)) {
        console.log(`还需 ${OPERATION_NAME} ${total}, 本次需 ${OPERATION_NAME} ${thisStepVolume}`)
        if (+thisStepVolume > 0) {
            reqMakeOrder({ ...argv, volume: thisStepVolume }, quote, unfinishedSteps)    
        }
    } else {
        const deltaVolume = +Number(thisStepVolume - currentVolume).toFixed(0);
        const contOperationName = makeOrderDirectionType(side, 0).n;
        console.log(`
            还需 ${OPERATION_NAME} ${total}, 本次需 ${OPERATION_NAME} ${thisStepVolume}, 
            持仓不足,
            需 ${OPERATION_NAME} ${currentVolume},
            ${contOperationName} ${deltaVolume},
        `)

        if (+currentVolume > 0) {
            reqMakeOrder({ ...argv, volume: currentVolume }, quote, unfinishedSteps)    
        } 

        if (+deltaVolume > 0) {
            reqMakeOrder({ ...argv, offset: 0, volume: deltaVolume }, quote, unfinishedSteps)    
        }
    }

    console.log(`============ 已完成执行 ${timeCount + 1} / ${steps} ==============`)    
    hasCancelOrderInThisLoop = false;
    dealedTimeCount = timeCount; //此时记录下来

})


function handleFinished () {
    console.log(`====================== 时间截止，交易结束 ======================`)
    secondsCounterTimer && clearInterval(secondsCounterTimer)
    reqTradingDataTimer && clearInterval(reqTradingDataTimer)
    process.exit(0)
}