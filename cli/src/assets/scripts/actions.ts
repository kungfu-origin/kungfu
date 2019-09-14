import { LOG_DIR } from '__gConfig/pathConfig';
import { setTimerPromiseTask, getLog } from '__gUtils/busiUtils';
import { listProcessStatus } from '__gUtils/processUtils';
import { getAccountList } from '__io/db/account';
import { getStrategyList } from '__io/db/strategy';
import { parseToString, dealStatus } from '@/assets/scripts/utils';
import { Observable, combineLatest, zip, concat } from 'rxjs';
import { mergeAll, map, concatAll } from 'rxjs/operators'
import logColor from '__gConfig/logColorConfig';

const colors = require('colors');
const path = require('path');

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

export const accountStrategyListStringify = (accounts: Account[], strategys: Strategy[]) => {
    return [
        ...accounts.map((a: Account): string => parseToString(
            [colors.cyan('account'), a.account_id],
            [8, 'auto'],
            1
        )),
        ...strategys.map((s: Strategy): string => parseToString(
            [colors.yellow('strategy'), s.strategy_id],
            [8, 'auto'],
            1
        ))
    ]
}

export const processStatusObservable = () => {
    return new Observable(subscriber => {
        setTimerPromiseTask(() => {
            return listProcessStatus()
                .then((processStatus: StringToStringObject) => {
                    subscriber.next(processStatus)
                })
                .catch((err: Error) => console.error(err))
        }, 2000)    
    })
}

export const accountListObservable = () => {
    return new Observable(subsciber => {
        setTimerPromiseTask(() => {
            return getAccountList()
                .then((accountList: Account[]) => {
                    subsciber.next(accountList)
                })
                .catch((err: Error) => console.error(err))
        }, 5000)
    })
}

export const strategyListObservable = () => {
    return new Observable(subsciber => {
        setTimerPromiseTask(() => {
            return getStrategyList()
            .then((strategyList: Strategy[]) => {
                subsciber.next(strategyList)
            })
            .catch((err: Error) => console.error(err))
        }, 5000)
    })
}

//系统所有进程列表
export const processListObservable = combineLatest(
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
            const strategyProcessId = `td_${b.strategy_id}`
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

const renderColoredProcessName = (processId: string) => {
    if(processId === 'master' || processId === 'ledger') 
        return colors.bold.underline.bgMagenta(processId)
    else if(processId.indexOf('td') !== -1) {
        return colors.bold.underline.cyan(processId)
    }
    else if(processId.indexOf('md') !== -1) {
        return colors.bold.underline.yellow(processId)
    }
    else 
        return colors.bold.underline.blue(processId)
}

const _dealLogMessage = (line: string, processId: string) => {
    let lineData: LogDataOrigin;
    try{
        lineData = JSON.parse(line);
    }catch(err){
        throw err;
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
    return new Observable(subscriber => {
        getLog(logPath, '', (line: string) => _dealLogMessage(line, pid))
        .then((logList: NumList) => subscriber.next(logList))
        .catch((err: Error) => subscriber.next(null))
        .finally(() => subscriber.complete())
    })
}


export const getMergedLogsObservable = (processIds: string[]) => {
    return zip(
        ...processIds
        .map((logPath: string) => {
            return getLogObservable(logPath)
        })
    )
}