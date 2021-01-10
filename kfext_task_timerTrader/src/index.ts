import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
import moment from 'moment';




const argv = minimist(process.argv.slice(2))
const { accountId, ticker, side, offset, volume, steps, triggerTime, finishTime } = argv;
const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const deltaTimestamp = Math.ceil((finishTime - triggerTime) / steps);
const tickers = ticker.toString().split(',')
    .map((t: string) => t.trim())
    .filter((t: string) => !!t)



console.log('===========================================')
console.log('[ARGS]: accountId', accountId, 'ticker', ticker, 'side', side, 'offset', offset, 'volume', volume, 'steps', steps, 'triggerTime', triggerTime, 'finishTime', finishTime)
console.log('===========================================')
console.log('TriggerTime', triggerTime, triggerTimeStr)
console.log('FinishTime', finishTime, finishTimeStr)
console.log('Executing every ', deltaTimestamp, 'ms')
console.log('Target Tickers ', tickers.join(','))
console.log('===========================================')

//行情request
const reqTradingDataTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'LEDGER_DATA'
        }
    })
}, 1000)

const TIMER_COUNT_OBSERVER = new Observable(subscriber => {
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
        if (currentCount >= 0 && currentCount <= steps - 1) {
            if (count !== currentCount) {
                subscriber.next(currentCount)
                count = +currentCount
            }
        }  
    }, 500)
})

const PROCESS_MSG_OBSERVER = new Observable(subscriber => {
    process.on('message', (packet) => {
        const { type, topic, data } = packet;
        if (type !== 'process:msg') return;
        subscriber.next({
            topic,
            data
        })
    })
})



const combineLatestObserver = combineLatest(
    TIMER_COUNT_OBSERVER,
    PROCESS_MSG_OBSERVER,
    (timeCount: number, proc: Proc) => {
        return {
            timeCount, 
            proc
        }       
    }
)

var dealedTimeCount: number = -1;
combineLatestObserver.subscribe((data: any) => {
    const { timeCount, proc } = data;
    console.log(timeCount, dealedTimeCount)

    if (timeCount <= dealedTimeCount) return;
    dealedTimeCount = timeCount;

    console.log(timeCount, dealedTimeCount, '---')    
    console.log(proc)
    
    
})
