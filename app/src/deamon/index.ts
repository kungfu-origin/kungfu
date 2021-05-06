import {
    buildTradingDataPipe,
    buildInstrumentsPipe,
    buildAllOrdersPipe,
    buildMarketDataPipe,
    buildKungfuGlobalDataPipe,
} from "__io/kungfu/tradingData";

import * as PM2_METHODS from './pm2Methods';

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

const { _pm2 } = require('__gUtils/processUtils');

_pm2.launchBus((err: Error, pm2_bus: any) => {

    if (err) {
        console.error(err)
    }

    pm2_bus.on('process:msg', (packet: any) => {
        const packetData = packet.data || {};
        const processData = packet.process || {};
        const pm2Id = processData.pm_id;
        const processName = processData.name;
        const dataType = packetData.type;
        const { 
            accountId, 
            ticker, 
            parentId, 
        } = packetData.body || {};

        switch (dataType) {
            case 'REQ_LEDGER_DATA':
                PM2_METHODS.resLedgerData(parentId, pm2Id, accountId, ticker, processName)
                break;
            case "REQ_QUOTE_DATA":
                PM2_METHODS.resQuoteData(pm2Id, ticker, processName)
                PM2_METHODS.resInstrumentInfo(pm2Id, ticker, processName)
                break;
            case "REQ_POS_ORDER_DATA":
                PM2_METHODS.resPosData(pm2Id, accountId, processName)
                PM2_METHODS.resOrderData(pm2Id, parentId, processName)
                break
            // case 'REQ_RECORD_DATA':
            //     const { data } = packetData.body;
            //     writeKungfu(processName, '', 'task', JSON.stringify(data))
        }
    })
})
