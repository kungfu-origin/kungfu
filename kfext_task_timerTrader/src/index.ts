import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import moment from 'moment';
import { transformArrayToObjectByKey, makeOrderDirectionType, buildTarget, reqMakeOrder } from './assets/utils';




const argv = minimist(process.argv.slice(2), {
    string: 'ticker',
})
const { ticker, side, offset, volume, steps, triggerTime, finishTime, parentId } = argv;
const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const deltaTimestamp = Math.ceil((finishTime - triggerTime) / steps);
const TICKER = ticker.toString().trim()
    
const TARGET_DIRECTION = makeOrderDirectionType(side, offset);
const TARGET_VOLUME = volume;
const LAST_STEP_COUNTE = steps - 1;



console.log('===========================================')
console.log('[ARGS]', process.argv.slice(2).join(','))
// console.log('[ARGS]: accountId', accountId, 'ticker', ticker, 'side', side, 'offset', offset, 'volume', volume, 'steps', steps, 'triggerTime', triggerTime, 'finishTime', finishTime)
console.log('===========================================')
console.log('TriggerTime', triggerTime, triggerTimeStr)
console.log('FinishTime', finishTime, finishTimeStr)
console.log('Executing every ', deltaTimestamp, 'ms')
console.log('Target Ticker ', TICKER)
console.log('===========================================')

//行情request
const reqTradingDataTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'REQ_LEDGER_DATA',
            parentId
        }
    })
}, 1000)

const TIMER_COUNT_OBSERVER = (): Observable<number> => new Observable((subscriber) => {
    let count: number = -1;
    let secondsCounterTimer = setInterval(() => {
        const currentTimestamp = moment().valueOf();
        
        if (currentTimestamp >= finishTime) {
            console.log('定时交易任务结束！')
            clearInterval(secondsCounterTimer)
            clearInterval(reqTradingDataTimer)
            return;
        }

        const currentCount = Math.ceil((currentTimestamp - triggerTime) / deltaTimestamp)
        if (currentCount >= 0 && currentCount <= LAST_STEP_COUNTE) {
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
        map((payload: ProcPayload): { [prop: string]: PosData } => {
            const { data } = payload;
            const { positions } = data;

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
    positionsPipe()
)

var dealedTimeCount: number = -1;
var targetPosData: any = null;

combineLatestObserver.subscribe(([timeCount, quotes, positions]: [ number, { [prop: string]: QuoteData }, { [prop: string]: PosData } ] ) => {

    // 判断是否可以交易
    

    if (timeCount <= dealedTimeCount) return;
    dealedTimeCount = timeCount;

    if (!targetPosData) {
        const pos = positions[`${TICKER}_${TARGET_DIRECTION}`] || {};
        //需保证在有持仓的情况下
        if (pos) {
            const { totalVolume } = pos;
            const totalVolumeResolved = totalVolume || 0;
            targetPosData = buildTarget({ 
                offset,
                side,
                ticker,
                totalVolume: totalVolumeResolved,
                targetVolume: TARGET_VOLUME
            })
        }
    }


    console.log('============ 正在执行 ', timeCount, `/ ${steps} ==============`)    


    const quote = quotes[TICKER];
    const pos = positions[`${TICKER}_${TARGET_DIRECTION}`] || {};
    const baseData = { 
        ...argv, 
        ticker: TICKER, 
        targetVolume: TARGET_VOLUME, 
        timeCount };

    if (timeCount !== 0) {
        //先撤单
    }
    
    reqMakeOrder(baseData, quote, pos)       

    //最后一步
    if (timeCount === LAST_STEP_COUNTE) {

        return;
    }

})
