import { startGetKungfuState } from '__gUtils/kungfuUtils';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';



export const KUNGFU_OBSERVER = new Observable(subscriber => {
    startGetKungfuState((state: any) => {
        subscriber.next(state)
    })
})   
       
const buildTradingDataPipe = () => {
    return KUNGFU_OBSERVER.pipe(
        map((data: any) => {
            const orders = Object.values(data.Order || {});

            return {
                orders  
            }
            

        })
    )
}



function insertOrders() {
    const { kungfu, watcher } = require('__gUtils/kungfuUtils');
    const { delayMiliSeconds } = require('__gUtils/BusiUtils');
    const longfist = kungfu.longfist;
    delayMiliSeconds(10000)
        .then(() => {
            console.log('insert orders');
            watcher.setup();
            watcher.step();
            for(let i = 1; i < 11; i++) {
                let order = longfist.Order();
                order.order_id = BigInt(i);
                order.update_time = BigInt(+new Date().getTime());
                order.account_id = '15014990';
                order.source_id = 'xtp'
                order.instrument_id = '60000' + i.toString();
                order.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
                order.volume_left = BigInt(+Number(500 * +Math.random()).toFixed(0));
                order.volume_traded = BigInt(order.volume - order.volume_left);
            
                watcher.publishState(order)
            }
        })
}

insertOrders()




