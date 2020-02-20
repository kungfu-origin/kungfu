import { startGetKungfuState } from '__gUtils/kungfuUtils';
import { Observable, from } from 'rxjs';
import { map } from 'rxjs/operators';
import { transformListToDataByAccountIdClientId, delayMiliSeconds } from '__gUtils/BusiUtils';



export const KUNGFU_OBSERVER = new Observable(subscriber => {
    startGetKungfuState((state: any) => {
        subscriber.next(state)
    })
})   
       
export const buildTradingDataPipe = () => {
    return KUNGFU_OBSERVER.pipe(
        map((data: any): any => {
            const orders = Object.values(data.Order || {});
            const trades = Object.values(data.Trade || {});
            const positions = Object.values(data.Position || {});

            return {
                orders: transformListToDataByAccountIdClientId(orders),
                trades: transformListToDataByAccountIdClientId(trades),
                positions: transformListToDataByAccountIdClientId(positions),
            }
        })
    )
}



async function insertTradingData() {
    const moment = require('moment')
    const { kungfu, watcher } = require('__gUtils/kungfuUtils');
    const { delayMiliSeconds } = require('__gUtils/BusiUtils');
    const longfist = kungfu.longfist;
    await delayMiliSeconds(10000)
    watcher.setup();
    await delayMiliSeconds(3000)
    watcher.step();
    await delayMiliSeconds(1000)

    console.log('insert trading data');

    const len = 3001;

    for(let i = 1; i < len; i++) {
       
        await delayMiliSeconds(100)

        let order = longfist.Order();
        order.order_id = BigInt(+moment().format('YYYYMMDD') + +i);
        order.update_time = BigInt(+new Date().getTime());
        order.account_id = '15014990';
        order.source_id = 'xtp';
        order.client_id = (i % 3 === 0) ? 'test' : '';
        order.instrument_id = '60000' + i.toString();
        order.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        order.volume_left = BigInt(+Number(500 * +Math.random()).toFixed(0));
        order.volume_traded = BigInt(order.volume - order.volume_left);
        order.status = i % 2;
        order.trading_day = moment().format('YYYYMMDD')
    
        watcher.publishState(order)

        let trade = longfist.Trade();
        trade.trade_id = BigInt(+moment().format('YYYYMMDD') + +i);
        trade.trade_time = BigInt(+new Date().getTime());
        trade.account_id = '15014990';
        trade.source_id = 'xtp';
        order.client_id = (i % 3 === 0) ? 'test' : '';
        trade.instrument_id = '60000' + i.toString();
        trade.price = +Number(1000 * +Math.random())
        trade.volume = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        trade.last_price = BigInt(+Number(10000 * +Math.random()).toFixed(0));
        trade.trading_day = moment().format('YYYYMMDD')

        watcher.publishState(trade)

        let position = longfist.Position();
        position.instrument_id = '60000' + i.toString();
        position.account_id = '15014990';
        position.source_id = 'xtp';
        position.direction = i % 2;
        position.trading_day = moment().format('YYYYMMDD')

        watcher.publishState(position)


        console.log('insert TradingData', `${i}/${len - 1}`)

    }

}

// insertTradingData()




