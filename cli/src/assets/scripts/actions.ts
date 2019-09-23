import { LOG_DIR } from '__gConfig/pathConfig';
import { setTimerPromiseTask, getLog, dealOrder, dealTrade, dealPos, dealAsset, delaySeconds } from '__gUtils/busiUtils';
import { addFile } from '__gUtils/fileUtils';
import { listProcessStatus } from '__gUtils/processUtils';
import { getAccountList, getAccountOrder, getAccountTrade, getAccountPos, getAccountAssetById } from '__io/db/account';
import { getStrategyList, getStrategyOrder, getStrategyTrade, getStrategyPos, getStrategyAssetById } from '__io/db/strategy';
import { buildTradingDataPipe, buildCashPipe } from '__io/nano/nanoSub';
import { switchMaster, switchLedger } from '__io/actions/base';
import { switchTd, switchMd } from '__io/actions/account';
import { switchStrategy } from '__io/actions/strategy';
import * as MSG_TYPE from '__io/nano/msgType';
import { parseToString, dealStatus, buildTargetDateRange } from '@/assets/scripts/utils';
import { Observable, combineLatest, forkJoin, merge, zip, concat } from 'rxjs';
import { map } from 'rxjs/operators';
import logColor from '__gConfig/logColorConfig';
import { logger } from '__gUtils/logUtils';

import moment from 'moment';

const colors = require('colors');
const path = require('path');
const { Tail } = require('tail');


export const switchProcess = (proc: any, messageBoard: any) =>{
    const status = proc.status === 'online';
    const startOrStop = !!status ? 'Stop' : 'Start';
    const startOrStopMaster = !!status ? 'Restart' : 'Start';
    switch(proc.type) {
        case 'main':
            if (proc.processId === 'master') {
                switchMaster(!status)
                .then(() => messageBoard.log(`${startOrStopMaster} Master process sucessfully!`, 2))
                .catch((err: Error) => logger.error(err))
            }
            else if(proc.processId === 'ledger') {
                switchLedger(!status)
                .then(() => messageBoard.log(`${startOrStop} Ledger process sucessfully!`, 2))
                .catch((err: Error) => logger.error(err))
            } 
            break
        case 'md':
            switchMd(proc, !status)
            .then(() => messageBoard.log(`${startOrStop} MD process sucessfully!`, 2))
            .catch((err: Error) => logger.error(err))
            break
        case 'td':
            switchTd(proc, !status)
            .then(() => messageBoard.log(`${startOrStop} TD process sucessfully!`, 2))
            .catch((err: Error) => logger.error(err))
            break;
        case 'strategy':
            switchStrategy(proc.processId, !status)
            .then(() => {messageBoard.log(`${startOrStop} Strategy process sucessfully!`, 2)})
            .catch((err: Error) => logger.error(err))
            break;
    }
}

export const getAccountsStrategys = async (): Promise<any> => {
    const getAccounts = getAccountList();
    const getStrategys = getStrategyList();
    const accounts = await getAccounts;
    const strategys = await getStrategys;
    return {
        accounts,
        strategys
    }
}

export const accountStrategyListStringify = (accounts: Account[], strategys: Strategy[], isTd?: boolean) => {
    return [
        ...accounts.map((a: Account): string => parseToString(
            [colors.cyan(isTd ? 'td' : 'account'), 
            isTd ? `td_${a.account_id}` : a.account_id],
            [8, 'auto'],
            1
        )),
        ...strategys.map((s: Strategy): string => parseToString(
            [colors.blue('strategy'), 
            s.strategy_id],
            [8, 'auto'],
            1
        ))
    ]
}

export const processStatusObservable = () => {
    return new Observable(observer => {
        observer.next({})
        setTimerPromiseTask(() => {
            return listProcessStatus()
                .then((processStatus: StringToStringObject) => {
                    observer.next(processStatus)
                })
                .catch((err: Error) => logger.error(err))
        }, 2000)    
    })
}

export const accountListObservable = () => {
    return new Observable(observer => {
        setTimerPromiseTask(() => {
            return getAccountList()
                .then((accountList: Account[]) => {
                    observer.next(accountList)
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

//系统所有进程列表
export const processListObservable = () => combineLatest(
    processStatusObservable(),
    accountListObservable(),
    strategyListObservable(),
    (processStatus: StringToStringObject, accountList: Account[], strategyList: Strategy[]) => {
        const mdList = accountList.filter((a: Account) => !!a.receive_md)
        const mdData = [{}, ...mdList].reduce((a: any, b: any): any => {
            const mdProcessId = `md_${b.source_name}`
            a[mdProcessId] = {
                ...b,
                processName: mdProcessId,
                processId: mdProcessId,
                typeName: colors.yellow('MD'),
                type: 'md',
                statusName: dealStatus(processStatus[mdProcessId]),
                status: processStatus[mdProcessId]
            };
            return a;
        })
        const tdData = [{}, ...accountList].reduce((a: any, b: any): any => {
            const tdProcessId = `td_${b.account_id}`
            a[tdProcessId] = {
                ...b,
                processName: tdProcessId,
                processId: tdProcessId,
                typeName: colors.cyan('TD'),
                type: 'td',
                statusName: dealStatus(processStatus[tdProcessId]),
                status: processStatus[tdProcessId]
            };
            return a;
        })
        const strategyData = [{}, ...strategyList].reduce((a: any, b: any): any => {
            const strategyProcessId = `${b.strategy_id}`
            a[strategyProcessId] = {
                ...b,
                processName: strategyProcessId,
                processId: strategyProcessId,
                typeName: colors.blue('Strat'),
                type: 'strategy',
                statusName: dealStatus(processStatus[strategyProcessId]),
                status: processStatus[strategyProcessId]
            };
            return a;
        })

        return [
            {
                processId: 'master',
                processName: colors.bold('MASTER'),
                typeName: colors.bgMagenta('Main'),
                type: 'main',
                statusName: dealStatus(processStatus['master']),
                status: processStatus['master']
            },
            {
                processId: 'ledger',
                processName: colors.bold('LEDGER'),
                typeName: colors.bgMagenta('Main'),
                type: 'main',
                statusName: dealStatus(processStatus['ledger']),
                status: processStatus['ledger']
            },
            ...Object.values(mdData),
            ...Object.values(tdData),
            ...Object.values(strategyData)
        ]
    }
)



// =============================== logs start ==================================================

const renderColoredProcessName = (processId: string) => {
    if(processId === 'master' || processId === 'ledger') {
        return colors.bold.underline.bgMagenta(processId)
    } 
    else if(processId.indexOf('td') !== -1) {
        return colors.bold.underline.cyan(processId)
    }
    else if(processId.indexOf('md') !== -1) {
        return colors.bold.underline.yellow(processId)
    }
    else {
        return colors.bold.underline.blue(processId)
    }
}

const dealLogMessage = (line: string, processId: string) => {
    let lineData: LogDataOrigin;
    try{
        lineData = JSON.parse(line);
    } catch (err) {
        return false
    }
    let messages = lineData.message.split('\n').filter((m: string) => m !== '');
    return messages.map((message: string) => {
        message = message.split('\n[')
            .join('[')
            .replace(/\[  info  \]/g, `[ ${colors[logColor.info]('info')}    ] [${renderColoredProcessName(processId)}]`)
            .replace(/\[ trace  \]/g, `[ trace   ] [${renderColoredProcessName(processId)}]`)
            .replace(/\[ error  \]/g, `[ ${colors[logColor.error]('error')}   ] [${renderColoredProcessName(processId)}]`)
            .replace(/\[warning \]/g, `[ ${colors[logColor.warning]('warning')} ] [${renderColoredProcessName(processId)}]`)
            .replace(/\[ debug  \]/g, `[ ${colors[logColor.debug]('debug')}   ] [${renderColoredProcessName(processId)}]`)
            .replace(/\[critical\]/g, `[ ${colors[logColor.critical]('critical')}] [${renderColoredProcessName(processId)}]`)
        if(
            message.indexOf('Failed') !== -1 ||
            message.indexOf('Traceback') !== -1 ||
            message.indexOf('critical') !== -1 ||
            message.indexOf('uncaught exception') !== -1
        ) message = `${colors[logColor.critical](message)}`
        
        return { 
            message,
            updateTime: lineData.timestamp
        }
    })
}


const getLogObservable = (pid: string) => {
    const logPath = path.join(LOG_DIR, `${pid}.log`);
    return new Observable(observer => {
        getLog(logPath, '', (line: string) => dealLogMessage(line, pid))
        .then((logList: NumList) => {
            observer.next(logList)
        })
        .catch((err: Error) => observer.next({
            list: []
        }))
        .finally(() => observer.complete())
    })
}


export const getMergedLogsObservable = (processIds: string[]) => {
    return forkJoin(
        ...processIds
        .map((logPath: string) => getLogObservable(logPath))        
    ).pipe(
        map((list: any[]) => {
            list = list
                .map((l: any) => l.list)
                .reduce((a: any, b: any): any => a.concat(b))
                .filter((l: any) => !!l)
                .map((l: any) => {
                    return l
                })
            if(list.length) {
                list = list
                    .sort((a: any, b: any) => moment(a.updateTime).valueOf() - moment(b.updateTime).valueOf())
                    .map((l: any) => l.message)  
            }
                          
            return list
        })
    )
}

const watchLogObservable = (processId: string) => {
    return new Observable(observer => {
        const logPath = path.join(LOG_DIR, `${processId}.log`);
        addFile('', logPath, 'file');
        const watcher = new Tail(logPath, {
            useWatchFile: true
        });
        watcher.watch();
        watcher.on('line', (line: string) => {
            const logList: any = dealLogMessage(line, processId);
            logList.forEach((l: any) => {
                observer.next(l.message || '')
            })
        })
        watcher.on('error', (err: Error) => {
            watcher.unwatch();
        })
    })
    
}

export const watchLogsObservable = (processIds: string[]) => {
    return merge(...processIds.map(pid => watchLogObservable(pid)))
}

export const LogsAndWatcherConcatObservable = (processIds: string[]) => {
    return concat(
        getMergedLogsObservable(processIds),
        watchLogsObservable(processIds)
    )
}

// =============================== logs end ==================================================

// =============================== trading Data start =========================================

export const getOrdersObservable = (type: string, id: string) => {    
    const getOrderMethods: StringToFunctionObject = {
        account: getAccountOrder,
        strategy: getStrategyOrder
    }
    const getOrderMethod = getOrderMethods[type]
    return new Observable(observer => {
        const dateRange = buildTargetDateRange();
        getOrderMethod(id,  { dateRange }).then((orders: OrderInputData[]) => {
            orders.forEach((order: OrderInputData) => {
                observer.next(['order', dealOrder(order)])
            })
        }).finally(() => observer.complete())
    })
}

export const getTradesObservable = (type: string, id: string) => {  
    const getTradeMethods: StringToFunctionObject = {
        account: getAccountTrade,
        strategy: getStrategyTrade
    }
    const getTradeMethod = getTradeMethods[type]
    return new Observable(observer => {
        const dateRange = buildTargetDateRange();
        getTradeMethod(id,  { dateRange }).then((trades: TradeInputData[]) => {
            trades.reverse().forEach((trade: TradeInputData) => {
                observer.next(['trade', dealTrade(trade)])
            })
        }).finally(() => observer.complete())
    })
}

export const getPosObservable = (type: string, id: string) => {
    const getPosMethods: StringToFunctionObject = {
        account: getAccountPos,
        strategy: getStrategyPos
    }
    const getPosMethod = getPosMethods[type]
    return new Observable(observer => {
        getPosMethod(id,  {}).then((positions: PosInputData[]) => {
            positions.forEach((pos: PosInputData) => {
                observer.next(['pos', dealPos(pos)]);
            })
        }).finally(() => observer.complete())
    })
}

interface TraderDataSub {
    msg_type: string;
    data: any;
}

export const tradingDataNanoObservable = (type: string, id: string) => {
    return new Observable(observer => {
        buildTradingDataPipe().subscribe((d: TraderDataSub) => {
            const msgType = d.msg_type;
            const tradingData = d.data;
            const { account_id, client_id } = tradingData; 
            const ledgerCategory = tradingData.ledger_category;
            let targetId: string = '';
            if(type === 'account') targetId = account_id
            else if (type === 'stratgy') targetId = client_id
            
            if(targetId !== id.toAccountId()) return;
            switch (+msgType) {
                case MSG_TYPE.order:
                    observer.next(['order', dealOrder(tradingData)]);
                    break
                case MSG_TYPE.trade:
                    observer.next(['trade', dealTrade(tradingData)]);
                    break
                case MSG_TYPE.position:
                    if(type === 'account' && ledgerCategory !== 0) return
                    if(type === 'strategy' && ledgerCategory !== 1) return
                    observer.next(['pos', dealPos(tradingData)]);
                    break
            }
        })
    })
}

export const getAssetObservable = (type: string, id: string) => {
    const getAssetMethods: StringToFunctionObject = {
        account: getAccountAssetById,
        strategy: getStrategyAssetById
    }
    const getAssetMethod = getAssetMethods[type]
    return new Observable(observer => {
        getAssetMethod(id).then((assets: AssetInputData[]) => {
            assets.forEach((asset: AssetInputData) => {
                const assetData = dealAsset(asset);
                if(type === 'account') delete assetData['clientId']
                else if(type === 'strategy') delete assetData['accountId']
                observer.next(['asset', assetData])
            })
        }).finally(() => observer.complete())
    })
}

export const AssetNanoObservable = (type: string, id: string) => {
    return new Observable(observer => {
        buildCashPipe().subscribe(({ data }: any) => {
            const { account_id, source_id, ledger_category, client_id } = data;
            if(type === 'account') {
                const accountId = `${source_id}_${account_id}`;                  
                if((ledger_category !== 0) || (accountId !== id)) return;
                const assetData = dealAsset(data);
                delete assetData['clientId']
                observer.next(['asset', assetData])
            } 
            else if(type === 'strategy') {
                if((ledger_category !== 1) || (client_id !== id)) return;
                const assetData = dealAsset(data);
                delete assetData['accountId']
                observer.next(['asset', assetData])
            }
        })
    })

}


export const tradingDataObservale = (type: string, id: string) => {
    return merge(
        getOrdersObservable(type, id),
        getTradesObservable(type, id),
        getPosObservable(type, id),
        tradingDataNanoObservable(type, id),
        getAssetObservable(type, id),
        AssetNanoObservable(type, id)
    )
}

// =============================== trading Data end =========================================


export const clearLog = () => {
    LOG_DIR
}