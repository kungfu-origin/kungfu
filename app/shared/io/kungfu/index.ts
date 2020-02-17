import {setTimerPromiseTask, delayMiliSeconds} from '__gUtils/busiUtils';
import {watcher, kungfu} from '__gUtils/kungfuUtils';

const longfist = kungfu.longfist;

// insertOrders();

watcher.run((w: any) => {
    console.log('watcher step', w.state);
});


function insertOrders() {
    console.log('insert orders');
    delayMiliSeconds(3000)
        .then(() => {
            watcher.setup();
            watcher.step();
            // for(let i = 1; i < 11; i++) {
            //     let order = longfist.Order();
            //     order.order_id = BigInt(i);
            //     order.account_id = '15014990';
            //     order.source_id = 'xtp'
            //     order.instrument_id = '60000' + i.toString();
            //     order.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
            //     order.volume_left = BigInt(+Number(500 * +Math.random()).toFixed(0));
            //     order.volume_traded = BigInt(order.volume - order.volume_left);
            //
            //     watcher.publishState(order)
            // }
        })

}


