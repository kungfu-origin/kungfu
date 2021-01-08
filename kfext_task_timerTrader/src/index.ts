import minimist from 'minimist';
import { Observable, combineLatest } from 'rxjs';
import moment from 'moment';




const argv = minimist(process.argv.slice(2))
const { accountId, ticker, side, offset, volume, steps, triggerTime, finishTime } = argv;
const triggerTimeStr = moment(triggerTime).format('YYYYMMDD HH:mm:ss');
const finishTimeStr = moment(finishTime).format('YYYYMMDD HH:mm:ss');
const deltaTimestamp = Math.ceil((finishTime - triggerTime) / steps);



console.log('===========================================')
console.log('[ARGS]: accountId', accountId, 'ticker', ticker, 'side', side, 'offset', offset, 'volume', volume, 'steps', steps, 'triggerTime', triggerTime, 'finishTime', finishTime)
console.log('===========================================')
console.log('triggerTime', triggerTime, triggerTimeStr)
console.log('finishTime', finishTime, finishTimeStr)
console.log('deltaTimestamp', deltaTimestamp)
console.log('===========================================')

//行情request
const reqTradingDataTimer = setInterval(() => {
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            type: 'MARKET_DATA'
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
    (timeCount: number, plist: Proc[]) => {
        return {
            timeCount, 
            plist
        }       
    }
)


combineLatestObserver.subscribe((data) => {
    
    console.log(data, '----')
})
