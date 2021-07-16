
import { dealStatus } from '@/assets/scripts/utils';


import { setTimerPromiseTask, resolveMemCpu } from '__gUtils/busiUtils';
import { logger } from '__gUtils/logUtils';
import { removeJournal } from '__gUtils/fileUtils';
import { listProcessStatusWithDetail, startArchiveMakeTask } from '__gUtils/processUtils';
import { KF_HOME } from '__gConfig/pathConfig';

import { switchMaster, switchCustomProcess } from '__io/actions/base';
import { switchTd, switchMd } from '__io/actions/account';
import { switchStrategy } from '__io/actions/strategy';

import { getTdList, getMdList } from '__io/kungfu/account';
import { getStrategyList } from '__io/kungfu/strategy';

import { Observable, combineLatest } from 'rxjs';
import { map } from 'rxjs/operators';

const colors = require('colors');

export const switchProcess = (proc: any, messageBoard: any, loading: any) =>{
    const status = proc.status !== '--';
    const startOrStop = !!status ? 'Stop' : 'Start';
    const { watcher } = require("__io/kungfu/watcher");

    if (!status) {
        if (proc.processId !== 'master') {
            if (!watcher.isLive()) {
                messageBoard.log(`Watcher is not alive! Start master and waitting for a while.`, 2)
                return 
            }
        }
    }

    switch(proc.type) {
        case 'main':
            if (proc.processId === 'master') {
                //开启，要归档, cli 需要clearjournal
                preSwitchMain(status, messageBoard, loading)
                    .then(() => {
                        loading.load(`${startOrStop} Master process`);
                        return switchMaster(!status)
                    })
                    .then(() => {
                        loading.stop();
                        return messageBoard.log(`${startOrStop} Master process success!`, 2)
                    })
                    .catch((err: Error) => logger.error(err))
            } 

            break
        case 'md':
            loading.load(`${startOrStop} MD process`)
            switchMd(proc, !status)
            .then(() => {
                loading.stop();
                return messageBoard.log(`${startOrStop} MD process success!`, 2)
            })
            .catch((err: Error) => logger.error(err))
            break
        case 'td':
            loading.load(`${startOrStop} TD process`)
            switchTd(proc, !status)
                .then(() => {
                    loading.stop();
                    return messageBoard.log(`${startOrStop} TD process success!`, 2)
                })
                .catch((err: Error) => logger.error(err))
            break;
        case 'strategy':
            loading.load(`${startOrStop} Strategy process`)
            switchStrategy(proc.processId, !status)
                .then(() => { 
                    loading.stop();
                    return messageBoard.log(`${startOrStop} Strategy process success!`, 2) 
                })
                .catch((err: Error) => logger.error(err))
            break;
        default:
            const processId = proc.processId
            loading.load(`${startOrStop} ${processId.toUpperCase()} process`)
            switchCustomProcess(!status, processId)
                .then(() => {
                    loading.stop();
                    return messageBoard.log(`${startOrStop} ${processId.toUpperCase()} process success!`, 2)
                })
                .catch((err: Error) => logger.error(err))
    }
}



export const processStatusObservable = () => {
    return new Observable(observer => {
        observer.next({})
        setTimerPromiseTask(() => {
            return listProcessStatusWithDetail()
                .then((processStatus: StringToProcessStatusDetail) => {
                    observer.next(processStatus)
                })
                .catch((err: Error) => logger.error(err))
        }, 1000)    
    })
}

export const mdTdStateObservable = () => {
    const { buildGatewayStatePipe } = require('__io/kungfu/tradingData');
    return buildGatewayStatePipe().pipe(
        map((item: any) => {
            
            let gatewayStatesData: StringToMdTdState = {};
            (item.gatewayStates || []).forEach((item: MdTdState) => {
                if (item.processId) {
                    gatewayStatesData[item.processId] = item;
                }
            });
            return gatewayStatesData 
        })
    )
}

//系统所有进程列表
export const processListObservable = () => combineLatest(
    processStatusObservable(),
    mdTdStateObservable(),
    mdListObservable(),
    tdListObservable(),
    strategyListObservable(),
    (
        processStatus: StringToProcessStatusDetail, 
        mdTdState: StringToMdTdState, 
        mdList: Md[],
        accountList: Td[], 
        strategyList: Strategy[]
    ) => {

        const mdData = [{}, ...mdList].reduce((a: any, b: any): any => {
            const mdProcessId = `md_${b.source_name}`
            a[mdProcessId] = {
                ...b,
                processName: mdProcessId,
                processId: mdProcessId,
                typeName: colors.yellow('MD'),
                type: 'md',
                statusName: dealStatus(buildTdMdStatus(mdProcessId, mdTdState, buildStatusDefault(processStatus[mdProcessId]).status)),
                status: buildStatusDefault(processStatus[mdProcessId]).status,
                monit: buildStatusDefault(processStatus[mdProcessId]).monit
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
                statusName: dealStatus(buildTdMdStatus(tdProcessId, mdTdState, buildStatusDefault(processStatus[tdProcessId]).status)),
                status: buildStatusDefault(processStatus[tdProcessId]).status,
                monit: buildStatusDefault(processStatus[tdProcessId]).monit
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
                statusName: dealStatus(buildStatusDefault(processStatus[strategyProcessId]).status),
                status: buildStatusDefault(processStatus[strategyProcessId]).status,
                monit: buildStatusDefault(processStatus[strategyProcessId]).monit
            };
            return a;
        })

        return [
            {
                processId: "archive",
                processName: colors.bold('_archive'),
                typeName: colors.bgMagenta('Main'),
                type: 'main',
                statusName: dealStatus(buildStatusDefault(processStatus['archive']).status),
                status: buildStatusDefault(processStatus['archive']).status,
                monit: buildStatusDefault(processStatus['archive']).monit
            },
            {
                processId: 'master',
                processName: colors.bold('MASTER'),
                typeName: colors.bgMagenta('Main'),
                type: 'main',
                statusName: dealStatus(buildStatusDefault(processStatus['master']).status),
                status: buildStatusDefault(processStatus['master']).status,
                monit: buildStatusDefault(processStatus['master']).monit
            },
            {
                processId: 'ledger',
                processName: colors.bold('LEDGER'),
                typeName: colors.bgMagenta('Main'),
                type: 'main',
                statusName: dealStatus(buildStatusDefault(processStatus['ledger']).status),
                status: buildStatusDefault(processStatus['ledger']).status,
                monit: buildStatusDefault(processStatus['ledger']).monit
            },

            {
                processId: 'bar',
                processName: colors.bold('BAR'),
                typeName: colors.green('Calc'),
                type: 'calc',
                statusName: dealStatus(buildStatusDefault(processStatus['bar']).status),
                status: buildStatusDefault(processStatus['bar']).status,
                monit: buildStatusDefault(processStatus['bar']).monit
            },
            ...Object.values(mdData || {}),
            ...Object.values(tdData || {}),
            ...Object.values(strategyData || {})
        ]
    }
)


function buildTdMdStatus(processId: string, stringMdTddState: StringToMdTdState, processStatus: string): string | number {
    if(!stringMdTddState[processId]) return processStatus
    else if(processStatus === 'online') return stringMdTddState[processId].state
    else return processStatus
}

function  buildStatusDefault(processStatus: ProcessStatusDetail | undefined) {
    if(!processStatus) return {
        status: '--',
        monit: {
            cpu: 0,
            memory: 0,
        }
    }


    const monit = processStatus.monit;
    const cpu = resolveMemCpu(monit, 'cpu');
    const memory = resolveMemCpu(monit, 'memory');
    return {
        status: processStatus.status,
        monit: {
            cpu: monit.cpu == 0 ? monit.cpu + '%' : colors.green(cpu),
            memory: monit.memory == 0 ? monit.memory + "M" : colors.green(memory)
        }
    }
}

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
                .then((tdList: Td[]) => {
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


function preSwitchMain (status: boolean, messageBoard: any, loading: any) {
    if (!status) {
        loading && loading.load(`Start Archive, Please wait...`, 2);
        return removeJournal(KF_HOME)
            .then(() => startArchiveMakeTask())
            .then(() => {
                loading && loading.stop();
                return messageBoard.log(`Archive success!`, 2)
        })
    }

    return Promise.resolve(true)
}