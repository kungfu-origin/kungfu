import { setTimerPromiseTask, delayMiliSeconds } from '__gUtils/busiUtils'; 
import { watcher, kungfu } from '__gUtils/kungfuUtils';

const longfist = kungfu.longfist;

watcher.setup();

insertOrders()

setTimerPromiseTask(() => {
    return new Promise((resolve) => {
        if (watcher.isLive()) {
            watcher.step();
            console.log('watcher.state: ', watcher.state);
        }
        resolve()
    })
}, 5000);



function insertOrders () {
    delayMiliSeconds(3000)
        .then(() => { 
            watcher.step()
            for(let i = 1; i < 11; i++) {
                let order = longfist.Order();
                order.order_id = BigInt(i);
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


