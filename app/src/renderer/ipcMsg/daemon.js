import { Subject } from 'rxjs';
import { filter, map } from 'rxjs/operators';

const { _pm2 } = require('__gUtils/processUtils');

const baseDaemonDataObserver = new Subject();

console.log('new baseDaemonDataObserver')

_pm2.launchBus((err, pm2_bus) => {
    if (err) {
        console.log(err)
    }
    pm2_bus.on('process:msg', (packet) => {  
        baseDaemonDataObserver.next(packet.data || {})
    })
})

export const buildTradingDataAccountPipeByDaemon = () => {
    return baseDaemonDataObserver
        .pipe(
            filter(packet => {
                const { type } = packet || {};
                return type === "DEAMON_TRADING_DATA_ACCOUNT"
            }),
            map(packet => {
                const { body } = packet || {};
                return body.data
            })
        )
}

export const buildTradingDataStrategyPipeByDaemon = () => {
    return baseDaemonDataObserver
        .pipe(
            filter(packet => {
                const { type } = packet || {};
                return type === "DEAMON_TRADING_DATA_STRATEGY"
            }),
            map(packet => {
                const { body } = packet || {};
                return body.data
            })
        )
}

export const buildInstrumentsPipeByDaemon = () => {
    return baseDaemonDataObserver
        .pipe(
            filter(packet => {
                const { type } = packet || {};
                return type === "DEAMON_INSTRUMENTS"
            }),
            map(packet => {
                const { body } = packet || {};
                return body.data
            })
        )
}

export const buildKungfuGlobalDataPipeByDaemon = () => {
    return baseDaemonDataObserver
        .pipe(
            filter(packet => {
                const { type } = packet || {};
                return type === "DEAMON_GLOBAL_DATA"
            }),
            map(packet => {
                const { body } = packet || {};
                return body.data
            })
        )
}

export const buildAllOrdersPipeByDaemon = () => {
    return baseDaemonDataObserver
    .pipe(
        filter(packet => {
            const { type } = packet || {};
            return type === "DEAMON_ALL_ORDERS"
        }),
        map(packet => {
            const { body } = packet || {};
            return body.data
        })
    )
}


export const buildMarketDataPipeByDaemon = () => {
    return baseDaemonDataObserver
    .pipe(
        filter(packet => {
            const { type } = packet || {};
            return type === "DEAMON_MARKET_DATA"
        }),
        map(packet => {
            const { body } = packet || {};
            return body.data
        })
    )
}

