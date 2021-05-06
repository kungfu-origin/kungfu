import {
    buildTradingDataPipe,
    buildInstrumentsPipe,
    buildAllOrdersPipe,
    buildMarketDataPipe,
    buildKungfuGlobalDataPipe,
    buildTaskDataPipe,
} from "__io/kungfu/tradingData";
import { watcher } from '__io/kungfu/watcher';
import { encodeKungfuLocation } from '__io/kungfu/kungfuUtils';

const { _pm2 } = require('__gUtils/processUtils');

_pm2.launchBus((err: Error, pm2_bus: any) => {
    pm2_bus.on('process:msg', (packet: any) => {
        const packetData = packet.data || {};
        const processData = packet.process || {};
        const pm2Id = processData.pm_id;
        const processName = processData.name;
        const dataType = packetData.type;
        const { 
            accountId, 
            exchangeId, 
            ticker, 
            parentId, 
            sourceId,  
        } = packetData.body || {};
    })
})


process.on('message', (packet: any) => {
    const { type, topic, data } = packet;        
    
})



buildTradingDataPipe('account').subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_TRADING_DATA_ACCOUNT",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})

buildTradingDataPipe('strategy').subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_TRADING_DATA_STRATEGY",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})


buildInstrumentsPipe().subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_INSTRUMENTS",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})

buildKungfuGlobalDataPipe().subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_GLOBAL_DATA",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})


buildAllOrdersPipe().subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_ALL_ORDERS",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})

buildMarketDataPipe().subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_MARKET_DATA",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})

buildTaskDataPipe().subscribe((data: any) => {
    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_TASK_DATA",
            body: {
                timestamp: new Date().getTime(),
                data,
            }
        }
    })
})


