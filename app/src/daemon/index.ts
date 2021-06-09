import {
    buildTradingDataPipe,
    buildMarketDataPipe,
    buildKungfuGlobalDataPipe,
} from "__io/kungfu/tradingData";

import { startGetKungfuWatcherStep, transformTradingItemListToData, dealQuote } from '__io/kungfu/watcher';

import * as PM2_METHODS from './pm2Methods';

startGetKungfuWatcherStep()

var QuotesRequiredInApp: TickerInTickerSet[] = [];

//daemon 要考虑的性能，daemon计算占用cpu/内存，传输序列化需要限制大小，同时避免在渲染进程做大量计算
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



buildMarketDataPipe().subscribe((data: any) => {
    const quotesAfterFilterKeys = QuotesRequiredInApp.map((item: TickerInTickerSet) => item.instrumentId)    
    const quotesAfterFilter =  QuotesRequiredInApp ? data.filter((item: QuoteOriginData) => {
        return quotesAfterFilterKeys.indexOf(item.instrument_id) !== -1;
    }) : [];

    const quotesResolved = quotesAfterFilter.map((item: QuoteOriginData) => dealQuote(item));

    //@ts-ignore
    process.send({
        type: "process:msg",
        data: {
            type: "DEAMON_MARKET_DATA",
            body: {
                timestamp: new Date().getTime(),
                data: transformTradingItemListToData(quotesResolved, 'quote'),
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
        const pm2Id = processData.pm_id || 0;
        const processName = processData.name || '';
        const dataType = packetData.type || '';
        const { 
            accountId, 
            ticker, 
            parentId, 
        } = packetData.body || {
            accountId: '', 
            ticker: '', 
            parentId: BigInt(0), 
        };

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
                break;
        }
    })
})


process.on('message', (packet) => {
    const { type, topic, data } = packet;        
    
    if (type !== 'process:msg')  return;
    switch (topic) {
        case "MAIN_RENDERER_SUBSCRIBED_TICKERS":
            QuotesRequiredInApp = data;
            break;
    }
})