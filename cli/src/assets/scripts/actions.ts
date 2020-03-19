import { LOG_DIR } from '__gConfig/pathConfig';
import { setTimerPromiseTask, getLog } from '__gUtils/busiUtils';
import { dealOrder, dealTrade, dealPos, dealAsset } from '__gUtils/kungfuUtils';
import { addFileSync } from '__gUtils/fileUtils';
import { listProcessStatusWithDetail } from '__gUtils/processUtils';
import { getTdList, getMdList, getAccountOrder, getAccountTrade, getAccountPos, getAccountAssetById } from '__io/kungfu/account';
import { getStrategyList, getStrategyOrder, getStrategyTrade, getStrategyPos, getStrategyAssetById } from '__io/kungfu/strategy';
// import { buildTradingDataPipe, buildCashPipe, buildGatewayStatePipe } from '__io/nano/nanoSub';
// import { nanoReqGatewayState } from '__io/nano/nanoReq';
import { switchMaster, switchLedger, switchCustomProcess } from '__io/actions/base';
import { switchTd, switchMd } from '__io/actions/account';
import { switchStrategy } from '__io/actions/strategy';
import * as MSG_TYPE from '__io/nano/msgType';
import { parseToString, dealStatus, buildTargetDateRange } from '@/assets/scripts/utils';
import { Observable, combineLatest, forkJoin, merge, concat } from 'rxjs';
import { map, combineAll } from 'rxjs/operators';
import logColor from '__gConfig/logColorConfig';
import { logger } from '__gUtils/logUtils';

const colors = require('colors');
const path = require('path');
const { Tail } = require('tail');
const moment = require('moment');


export const switchProcess = (proc: any, messageBoard: any) =>{
    const status = proc.status === 'online';
    const startOrStop = !!status ? 'Stop' : 'Start';
    const startOrStopMaster = !!status ? 'Restart' : 'Start';
    switch(proc.type) {
        case 'main':
            if (proc.processId === 'master') {
                switchMaster(!status)
                .then(() => messageBoard.log(`${startOrStopMaster} Master process success!`, 2))
                .catch((err: Error) => logger.error(err))
            }
            else if(proc.processId === 'ledger') {
                switchLedger(!status)
                .then(() => messageBoard.log(`${startOrStop} Ledger process success!`, 2))
                .catch((err: Error) => logger.error(err))
            } 
            break
        case 'md':
            switchMd(proc, !status)
            .then(() => messageBoard.log(`${startOrStop} MD process success!`, 2))
            .catch((err: Error) => logger.error(err))
            break
        case 'td':
            switchTd(proc, !status)
            .then(() => messageBoard.log(`${startOrStop} TD process success!`, 2))
            .catch((err: Error) => logger.error(err))
            break;
        case 'strategy':
            switchStrategy(proc.processId, !status)
            .then(() => {messageBoard.log(`${startOrStop} Strategy process success!`, 2)})
            .catch((err: Error) => logger.error(err))
            break;
        default:
            switchCustomProcess(!status, proc)
            .then(() => messageBoard.log(`${startOrStop} ${proc.toUpperCase()} process success!`, 2))
            .catch((err: Error) => logger.error(err))
    }
}




// const mdTdStateObservable = () => {
//     nanoReqGatewayState();
//     let mdTdStateData: StringToMdTdState = {};
//     return new Observable(observer => {
//         observer.next({})
//         buildGatewayStatePipe().subscribe((data: any[] | undefined): void => {
//             data = data || []
//             mdTdStateData[data[0]] = data[1]
//             observer.next(mdTdStateData)
//         })
//     })
// }

// export const processStatusObservable = () => {
//     return new Observable(observer => {
//         observer.next({})
//         setTimerPromiseTask(() => {
//             return listProcessStatusWithDetail()
//                 .then((processStatus: StringToProcessStatusDetail) => {
//                     observer.next(processStatus)
//                 })
//                 .catch((err: Error) => logger.error(err))
//         }, 1000)    
//     })
// }

export const mdListObservable = () => {
    return new Observable(observer => {
        setTimerPromiseTask(() => {
            return getMdList()
                .then((mdList: Md[]) => {
                    observer.next(mdList)
                })
                .catch((err: Error) => logger.error(err))
        }, 5000)
    })
}

export const tdListObservable = () => {
    return new Observable(observer => {
        setTimerPromiseTask(() => {
            return getTdList()
                .then((tdList: Account[]) => {
                    observer.next(tdList)
                })
                .catch((err: Error) => logger.error(err))
        }, 5000)
    })
}

export const strategyListObservable = () => {
    return new Observable(observer => {
        setTimerPromiseTask(() => {
            return getStrategyList()
            .then((strategyList: Strategy[]) => {
                observer.next(strategyList)
            })
            .catch((err: Error) => logger.error(err))
        }, 5000)
    })
}


// //系统所有进程列表
// export const processListObservable = () => combineLatest(
//     processStatusObservable(),
//     mdTdStateObservable(),
//     mdListObservable(),
//     tdListObservable(),
//     strategyListObservable(),
//     (
//         processStatus: StringToProcessStatusDetail, 
//         mdTdState: StringToMdTdState, 
//         mdList: Md[],
//         accountList: Account[], 
//         strategyList: Strategy[]
//     ) => {
//         const mdData = [{}, ...mdList].reduce((a: any, b: any): any => {
//             const mdProcessId = `md_${b.source_name}`
//             a[mdProcessId] = {
//                 ...b,
//                 processName: mdProcessId,
//                 processId: mdProcessId,
//                 typeName: colors.yellow('MD'),
//                 type: 'md',
//                 statusName: dealStatus(buildTdMdStatus(mdProcessId, mdTdState, buildStatusDefault(processStatus[mdProcessId]).status)),
//                 status: buildStatusDefault(processStatus[mdProcessId]).status,
//                 monit: buildStatusDefault(processStatus[mdProcessId]).monit
//             };
//             return a;
//         })
//         const tdData = [{}, ...accountList].reduce((a: any, b: any): any => {
//             const tdProcessId = `td_${b.account_id}`
//             a[tdProcessId] = {
//                 ...b,
//                 processName: tdProcessId,
//                 processId: tdProcessId,
//                 typeName: colors.cyan('TD'),
//                 type: 'td',
//                 statusName: dealStatus(buildTdMdStatus(tdProcessId, mdTdState, buildStatusDefault(processStatus[tdProcessId]).status)),
//                 status: buildStatusDefault(processStatus[tdProcessId]).status,
//                 monit: buildStatusDefault(processStatus[tdProcessId]).monit
//             };
//             return a;
//         })
//         const strategyData = [{}, ...strategyList].reduce((a: any, b: any): any => {
//             const strategyProcessId = `${b.strategy_id}`
//             a[strategyProcessId] = {
//                 ...b,
//                 processName: strategyProcessId,
//                 processId: strategyProcessId,
//                 typeName: colors.blue('Strat'),
//                 type: 'strategy',
//                 statusName: dealStatus(buildStatusDefault(processStatus[strategyProcessId]).status),
//                 status: buildStatusDefault(processStatus[strategyProcessId]).status,
//                 monit: buildStatusDefault(processStatus[strategyProcessId]).monit
//             };
//             return a;
//         })

//         return [
//             {
//                 processId: 'master',
//                 processName: colors.bold('MASTER'),
//                 typeName: colors.bgMagenta('Main'),
//                 type: 'main',
//                 statusName: dealStatus(buildStatusDefault(processStatus['master']).status),
//                 status: buildStatusDefault(processStatus['master']).status,
//                 monit: buildStatusDefault(processStatus['master']).monit
//             },
//             {
//                 processId: 'ledger',
//                 processName: colors.bold('LEDGER'),
//                 typeName: colors.bgMagenta('Main'),
//                 type: 'main',
//                 statusName: dealStatus(buildStatusDefault(processStatus['ledger']).status),
//                 status: buildStatusDefault(processStatus['ledger']).status,
//                 monit: buildStatusDefault(processStatus['ledger']).monit
//             },
//             {
//                 processId: 'bar',
//                 processName: colors.bold('BAR'),
//                 typeName: colors.green('Calc'),
//                 type: 'calc',
//                 statusName: dealStatus(buildStatusDefault(processStatus['bar']).status),
//                 status: buildStatusDefault(processStatus['bar']).status,
//                 monit: buildStatusDefault(processStatus['bar']).monit
//             },
//             ...Object.values(mdData || {}),
//             ...Object.values(tdData || {}),
//             ...Object.values(strategyData || {})
//         ]
//     }
// )

// function buildTdMdStatus(processId: string, stringMdTddState: StringToMdTdState, processStatus: string): string | number {
//     if(!stringMdTddState[processId]) return processStatus
//     else if(processStatus === 'online') return stringMdTddState[processId].state
//     else return processStatus
// }

// function  buildStatusDefault(processStatus: ProcessStatusDetail | undefined) {
//     if(!processStatus) return {
//         status: '--',
//         monit: {
//             cpu: 0,
//             memory: 0,
//         }
//     }

//     const memory = Number(BigInt((processStatus.monit || {}).memory || 0) / BigInt(1024 * 1024));
//     const cpu =  (processStatus.monit || {}).cpu || 0
//     return {
//         status: processStatus.status,
//         monit: {
//             cpu: cpu == 0 ? cpu : colors.green(cpu),
//             memory: memory == Number(0) ? memory : colors.green(memory)
//         }
//     }
// }


// // =============================== logs start ==================================================

// const dealUpdateTime = (updateTime: string): any => {
//     const updateTimeLength = updateTime.length;
//     if(updateTimeLength === 24) return updateTime.slice(0, 18)
//     else if(updateTime.split('-').length > 1) {
//         const newUpdateTime = updateTime.split('-').slice(1).join('/');
//         return dealUpdateTime(newUpdateTime)
//     }
//     else return updateTime
// }

// const dealLogMessage = (line: string, processId: string) => {
//     let lineData: LogDataOrigin;
//     try{
//         lineData = JSON.parse(line);
//     } catch (err) {
//         return false
//     }
//     let messages = lineData.message.split('\n').filter((m: string) => m !== '');
//     return messages.map((message: string) => {
//         message = message.split('\n[').join('[')

//         if(message.split('[').length < 4) {
//             const updateTime = moment(lineData.timestamp).format('MM/DD hh:mm:ss.000');
//             const type = 'error'.includes(lineData.type) ? 'error' : lineData.type;
//             message = `[${updateTime}] [ ${type}  ] ${message.trim()}`
//         }

//         const msgList = message.split(']');
//         const updateTime = msgList[0].slice(1)
//         const updateTimeResolve = dealUpdateTime(updateTime)
//         const typeResolve = `[${msgList[1].trim().slice(1).trim()}]`;
        
//         let lastInfo = ''
//         if(msgList.length >= 4) {
//             lastInfo = msgList.slice(4).join(']')
//         } else {
//             lastInfo = msgList.slice(2).join(']')
//         }

//         const messageResolve = `[${updateTimeResolve}]${typeResolve}${lastInfo}`

//         message = messageResolve
//             .replace(/\[info\]/g, `[ ${colors[logColor.info]('info')}    ] `)
//             .replace(/\[out\]/g,    `[ out     ] `)
//             .replace(/\[trace\]/g, `[ trace   ] `)
//             .replace(/\[error\]/g, `[ ${colors[logColor.error]('error')}   ] `)
//             .replace(/\[warning\]/g, `[ ${colors[logColor.warning]('warning')} ] `)
//             .replace(/\[debug\]/g, `[ ${colors[logColor.debug]('debug')}   ] `)
//             .replace(/\[critical\]/g, `[ ${colors[logColor.critical]('critical')}] `)
    
//         if(message.includes('critical')) message = `${colors[logColor.critical](message)}`
        
//         return { 
//             message,
//             updateTime: lineData.timestamp
//         }
//     })
// }


// const getLogObservable = (pid: string) => {
//     const logPath = path.join(LOG_DIR, `${pid}.log`);
//     return new Observable(observer => {
//         getLog(logPath, '', (line: string) => dealLogMessage(line, pid))
//         .then((logList: NumList) => observer.next(logList))
//         .catch(() => observer.next({ list: [] }))
//         .finally(() => observer.complete())
//     })
// }


// export const getMergedLogsObservable = (processIds: string[]) => {
//     return forkJoin(
//         ...processIds
//         .map((logPath: string) => getLogObservable(logPath))        
//     ).pipe(
//         map((list: any[]) => {
//             list = list
//                 .map((l: any) => l.list)
//                 .reduce((a: any, b: any): any => a.concat(b))
//                 .filter((l: any) => !!l)
//                 .map((l: any) => {
//                     return l
//                 })
//             if(list.length) {
//                 list = list
//                     .sort((a: any, b: any) => moment(a.updateTime).valueOf() - moment(b.updateTime).valueOf())
//                     .map((l: any) => l.message)  
//             }
                          
//             return list
//         })
//     )
// }

// const watchLogObservable = (processId: string) => {
//     return new Observable(observer => {
//         const logPath = path.join(LOG_DIR, `${processId}.log`);
//         addFileSync('', logPath, 'file');
//         const watcher = new Tail(logPath, {
//             useWatchFile: true
//         });
//         watcher.watch();
//         watcher.on('line', (line: string) => {
//             const logList: any = dealLogMessage(line, processId);
//             logList.kfForEach((l: any) => observer.next(l.message || ''))
//         })
//         watcher.on('error', () => watcher.unwatch())
//     })
    
// }

// export const watchLogsObservable = (processIds: string[]) => {
//     return merge(...processIds.map(pid => watchLogObservable(pid)))
// }

// export const LogsAndWatcherConcatObservable = (processIds: string[]) => {
//     return concat(
//         getMergedLogsObservable(processIds),
//         watchLogsObservable(processIds)
//     )
// }

// // =============================== logs end ==================================================

// // =============================== trading Data start =========================================

// export const getOrdersObservable = (type: string, id: string) => {    
//     const getOrderMethods: StringToFunctionObject = {
//         account: getAccountOrder,
//         strategy: getStrategyOrder
//     }
//     const getOrderMethod = getOrderMethods[type]
//     return new Observable(observer => {
//         const dateRange = buildTargetDateRange();
//         getOrderMethod(id,  { dateRange })
//             .then((orders: OrderInputData[]) => {
//                 logger.info('orders ', orders.length)
//                 const ordersResolve = orders.slice(0, 1000).map((order: OrderInputData) => {
//                     return dealOrder(order)
//                 })
//                 observer.next(['orderList', ordersResolve])
//             })
//             .catch((err: Error) => logger.error('[ORDER GET]', err))
//             .finally(() => observer.complete())
//     })
// }

// export const getTradesObservable = (type: string, id: string) => {  
//     const getTradeMethods: StringToFunctionObject = {
//         account: getAccountTrade,
//         strategy: getStrategyTrade
//     }
//     const getTradeMethod = getTradeMethods[type]
//     return new Observable(observer => {
//         const dateRange = buildTargetDateRange();
//         getTradeMethod(id,  { dateRange })
//             .then((trades: TradeInputData[]) => {
//                 logger.info('trades ', trades.length)
//                 const tradesResolve = trades.reverse().slice(0, 1000).map((trade: TradeInputData) => {
//                     return dealTrade(trade)
//                 })
//                 observer.next(['tradeList', tradesResolve])
//             })
//             .catch((err: Error) => logger.error('[TRADE GET]', err))            
//             .finally(() => observer.complete())
//     })
// }

// export const getPosObservable = (type: string, id: string) => {
//     const getPosMethods: StringToFunctionObject = {
//         account: getAccountPos,
//         strategy: getStrategyPos
//     }
//     const getPosMethod = getPosMethods[type]
//     return new Observable(observer => {
//         getPosMethod(id,  {})
//             .then((positions: PosInputData[]) => {
//                 logger.info('pos ', positions.length)
//                 positions.kfForEach((pos: PosInputData) => {
//                     observer.next(['pos', dealPos(pos)]);
//                 })
//             })
//             .catch((err: Error) => logger.error('[POS GET]', err))
//             .finally(() => observer.complete())
//     })
// }

// interface TraderDataSub {
//     msg_type: string;
//     data: any;
// }

// export const tradingDataNanoObservable = (type: string, id: string) => {
//     return new Observable(observer => {
//         buildTradingDataPipe().subscribe((d: TraderDataSub) => {
//             const msgType = d.msg_type;
//             const tradingData = d.data;
//             const { account_id, client_id } = tradingData; 
//             const ledgerCategory = tradingData.ledger_category;
//             const targetId = type === 'account' ? account_id : client_id;
//             const currentId = type === 'account' ? id.toAccountId() : id;
    
//             if(targetId !== currentId) return;
//             switch (+msgType) {
//                 case MSG_TYPE.order:
//                     observer.next(['order', dealOrder(tradingData)]);
//                     break
//                 case MSG_TYPE.trade:
//                     observer.next(['trade', dealTrade(tradingData)]);
//                     break
//                 case MSG_TYPE.position:
//                     if(type === 'account' && ledgerCategory !== 0) return
//                     if(type === 'strategy' && ledgerCategory !== 1) return
//                     observer.next(['pos', dealPos(tradingData)]);
//                     break
//             }
//         })
//     })
// }

// export const getAssetObservable = (type: string, id: string) => {
//     const getAssetMethods: StringToFunctionObject = {
//         account: getAccountAssetById,
//         strategy: getStrategyAssetById
//     }
//     const getAssetMethod = getAssetMethods[type]
//     return new Observable(observer => {
//         getAssetMethod(id).then((assets: AssetInputData[]) => {
//             logger.info('asset ', assets.length)
//             assets.kfForEach((asset: AssetInputData) => {
//                 const assetData = dealAsset(asset);
//                 if(type === 'account') delete assetData['clientId']
//                 else if(type === 'strategy') delete assetData['accountId']
//                 observer.next(['asset', assetData])
//             })
//         })
//         .catch((err: Error) => logger.error('[ASSET GET]', err))        
//         .finally(() => observer.complete())
//     })
// }

// export const AssetNanoObservable = (type: string, id: string) => {
//     return new Observable(observer => {
//         buildCashPipe().subscribe(({ data }: any) => {
//             const { account_id, source_id, ledger_category, client_id } = data;
//             if(type === 'account') {
//                 const accountId = `${source_id}_${account_id}`;                  
//                 if((ledger_category !== 0) || (accountId !== id)) return;
//                 const assetData = dealAsset(data);
//                 delete assetData['clientId']
//                 observer.next(['asset', assetData])
//             } 
//             else if(type === 'strategy') {
//                 if((ledger_category !== 1) || (client_id !== id)) return;
//                 const assetData = dealAsset(data);
//                 delete assetData['accountId']
//                 observer.next(['asset', assetData])
//             }
//         })
//     })

// }


// export const tradingDataObservale = (type: string, id: string) => {
//     return merge(
//         concat(
//             getOrdersObservable(type, id),
//             getTradesObservable(type, id),
//             getPosObservable(type, id),
//             getAssetObservable(type, id),
//         ),
//         AssetNanoObservable(type, id),
//         tradingDataNanoObservable(type, id),
//     )
// }

// // =============================== trading Data end =========================================


// export const clearLog = () => {
//     LOG_DIR
// }