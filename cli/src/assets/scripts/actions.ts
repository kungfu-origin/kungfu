import { setTimerPromiseTask } from '__gUtils/busiUtils';
import { listProcessStatus } from '__gUtils/processUtils';
import { getAccountList } from '__io/db/account';
import { getStrategyList } from '__io/db/strategy';
import { parseToString, dealStatus } from '@/assets/scripts/utils';
import { Observable, combineLatest } from 'rxjs';

const colors = require('colors');

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


