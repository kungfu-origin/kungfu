import logColor from '__gConfig/logColorConfig';
import { buildProcessLogPath } from '__gConfig/pathConfig';
import { addFileSync } from '__gUtils/fileUtils';
import { getLog } from '__gUtils/busiUtils';


import { Observable, forkJoin, merge, concat } from 'rxjs';
import { map } from 'rxjs/operators';


const path = require('path');
const { Tail } = require('tail');
const moment = require('moment');
const colors = require('colors');

var logWather: any = null;

// // =============================== logs start ==================================================

const dealUpdateTime = (updateTime: string): any => {
    const updateTimeLength = updateTime.length;
    if(updateTimeLength === 24) return updateTime.slice(0, 18)
    else if(updateTime.split('-').length > 1) {
        const newUpdateTime = updateTime.split('-').slice(1).join('/');
        return dealUpdateTime(newUpdateTime)
    }
    else return updateTime
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
        message = message.split('\n[').join('[')

        if(message.split('[').length < 4) {
            const updateTime = `[${moment(lineData.timestamp).format('MM/DD HH:mm:ss.000')}]`;
            const type = 'error'.includes(lineData.type) ? 'error' : lineData.type;
            message = `${updateTime} [ ${type}  ] ${message.trim()}`
        }

        const msgList = message.split(']');
        const updateTime = msgList[0].slice(1)
        const updateTimeResolve = `[${dealUpdateTime(updateTime)}]`
        const typeResolve = `[${msgList[1].trim().slice(1).trim()}]`;
        
        let lastInfo = ''
        if(msgList.length >= 4) {
            lastInfo = msgList.slice(4).join(']')
        } else {
            lastInfo = msgList.slice(2).join(']')
        }

        const messageResolve = `${colors.cyan(updateTimeResolve)} ${typeResolve} ${lastInfo}`

        message = messageResolve
            .replace(/\[info\]/g, `[ ${colors[logColor.info]('info')}    ] `)
            .replace(/\[out\]/g,    `[ out     ] `)
            .replace(/\[trace\]/g, `[ trace   ] `)
            .replace(/\[error\]/g, `[ ${colors[logColor.error]('error')}   ] `)
            .replace(/\[warning\]/g, `[ ${colors[logColor.warning]('warning')} ] `)
            .replace(/\[debug\]/g, `[ ${colors[logColor.debug]('debug')}   ] `)

        const isCritical = message.includes('critical');
    
        if(isCritical) message = `${colors[logColor.critical](message)}`
        
        return { 
            message,
            isCritical,
            updateTime: lineData.timestamp
        }
    })
}


const getLogObservable = (pid: string) => {
    const logPath = buildProcessLogPath(pid)
    return new Observable(observer => {
        getLog(logPath, '', (line: string) => dealLogMessage(line, pid))
        .then((logList: any) => observer.next(logList))
        .catch(() => observer.next({ list: [] }))
        .finally(() => observer.complete())
    })
}


export const getMergedLogsObservable = (processIds: string[], boardWidth: number) => {
    return forkJoin(
        ...processIds
        .map((logPath: string) => getLogObservable(logPath))        
    ).pipe(
        map((list: any[]): any[] => {

            let listResolved: any = []

            list = list
                .map((l: any) => l.list)
                .reduce((a: any, b: any): any => a.concat(b))
                .filter((l: any) => !!l)
                .map((l: any) => {
                    return l
                })

            if(list.length) {
                list
                .sort((a: any, b: any) => moment(a.updateTime).valueOf() - moment(b.updateTime).valueOf())
                .map((l: any) => l)  
                .forEach((l: any) => {
                    const message = l.message;
                    const isCritical = l.isCritical;

                    if (message.length < boardWidth) {
                        listResolved.push(message)
                    } else {
                        splitStrByLength(message, boardWidth).forEach(splitLine => {
                            if (isCritical) {
                                listResolved.push(colors.red(splitLine))
                            } else {
                                listResolved.push(splitLine)
                            }
                        })
                    }
                })
                    
            }       
            return listResolved
        })
    )
}

const watchLogObservable = (processId: string, boardWidth: number) => {
    logWather && (logWather.unwatch());
    return new Observable(observer => {
        const logPath = buildProcessLogPath(processId)
        addFileSync('', logPath, 'file');
        const watcher = new Tail(logPath, {
            useWatchFile: true
        });

        logWather = watcher;
        
        watcher.watch();
        watcher.on('line', (line: string) => {
            const logList: any = dealLogMessage(line, processId);

            logList.kfForEach((l: any) => {
                if (l.message.length < boardWidth) {
                    observer.next(l.message || '')                    
                } else {
                    splitStrByLength(l.message, boardWidth).forEach(m => {
                        const isCritical = l.isCritical;

                        if (isCritical) {
                            observer.next(colors.red(m || ''))
                        } else {
                            observer.next(m || '')
                        }
                    })
                }
            })
        })
        watcher.on('error', () => watcher.unwatch())
    })
    
}

export const watchLogsObservable = (processIds: string[], boardWidth: number) => {
    return merge(...processIds.map(pid => watchLogObservable(pid, boardWidth)))
}

export const LogsAndWatcherConcatObservable = (processIds: string[], boardWidth: number) => {
    return concat(
        getMergedLogsObservable(processIds, boardWidth),
        watchLogsObservable(processIds, boardWidth)
    )
}


function splitStrByLength (str: string, len: number): String[] {

    const strLen = str.length;
    const groupNum = Math.ceil(strLen / len);
    
    let i = 0, strList = []; 
    for (i; i < groupNum; i++) {
        const start = i * len;
        const end = (i + 1) * len;
        strList.push(str.slice(start, end).trim())
    }

    return strList
}

// // =============================== logs end ==================================================


