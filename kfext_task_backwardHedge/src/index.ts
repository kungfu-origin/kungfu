import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import { getCurrentCount, getCurrentTimestamp } from './assets/utils';
import moment from 'moment';


const argv = minimist(process.argv.slice(2), {
    string: ['ticker', 'index']
})

const { ticker, tickerExchangeId, tickerMd, index, indexExchangeId, indexMd, side, offset, volume, steps, triggerTime, finishTime, interval, parentId, accountId, lastSingularity, lastSingularityMilliSecond, maxLotByStep } = argv;

const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const LOOP_INTERVAL = interval;
const TICKER = ticker.toString().trim();


console.log('==================== 交易信息 =======================')
console.log('[ARGS]', process.argv.slice(2).join(','))
console.log('===================================================')
console.log('[开始时间]', triggerTime, triggerTimeStr)
console.log('[结束时间]', finishTime, finishTimeStr)
console.log('[执行间隔]', LOOP_INTERVAL, 'ms')
console.log('[目标标的] ', TICKER)
console.log('===================================================')

//获取当月/下月/当季/下季
const T_M_TICKER = TICKER;
const N_M_TICKER = TICKER


//@ts-ignore
process.send({
    type: 'process:msg',
    data: {
        type: 'SUBSCRIBE_BY_TICKER',
        body: {
            ticker: TICKER,
            exchangeId: tickerExchangeId,
            sourceId: tickerMd
        }
    }
})

console.log(`[订阅] 合约 ${index} ${tickerExchangeId} ${accountId}`)

//@ts-ignore
process.send({
    type: 'process:msg',
    data: {
        type: 'SUBSCRIBE_BY_TICKER',
        body: {
            ticker: index,
            exchangeId: indexExchangeId,
            sourceId: indexMd
        }
    }
})

console.log(`[订阅] 指数 ${index} ${indexExchangeId} ${accountId}`)


var reqQuoteTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: "REQ_QUOTE",
            body: {
                ticker: TICKER
            }
        }
    })
}, 200)

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