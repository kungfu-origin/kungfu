import {
    buildTradingDataPipe,
    buildInstrumentsPipe,
    buildAllOrdersPipe,
    buildMarketDataPipe,
    buildKungfuGlobalDataPipe,
    buildTaskDataPipe,
} from "__io/kungfu/tradingData";


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
