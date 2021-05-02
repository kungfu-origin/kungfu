import { Observable } from 'rxjs';
import { filter, map } from 'rxjs/operators';

const { _pm2 } = require('__gUtils/processUtils');

const baseDeamonDataObserver = new Observable(subscriber => {
    subscriber.next({})
    _pm2.launchBus((err, pm2_bus) => {
        if (err) {
            console.log(err)
        }
        pm2_bus.on('process:msg', (packet) => {  
            subscriber.next(packet.data || {})
        })
    })
})

export const buildTradingDataAccountPipeByDeamon = () => {
    return baseDeamonDataObserver
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

export const buildTradingDataStrategyPipeByDeamon = () => {
    return baseDeamonDataObserver
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

export const buildInstrumentsPipeByDeamon = () => {
    return baseDeamonDataObserver
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

export const buildKungfuGlobalDataPipeByDeamon = () => {
    return baseDeamonDataObserver
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

export const buildAllOrdersPipeByDeamon = () => {
    return baseDeamonDataObserver
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


export const buildMarketDataPipeByDeamon = () => {
    return baseDeamonDataObserver
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

export const buildTaskDataPipeByDeamon = () => {
    return baseDeamonDataObserver
    .pipe(
        filter(packet => {
            const { type } = packet || {};
            return type === "DEAMON_TASK_DATA"
        }),
        map(packet => {
            const { body } = packet || {};
            return body.data
        })
    )
}