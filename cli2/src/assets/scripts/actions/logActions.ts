import logColor from '__gConfig/logColorConfig';
import { LOG_DIR } from '__gConfig/pathConfig';
import { addFileSync } from '__gUtils/fileUtils';
import { getLog } from '__gUtils/busiUtils';


import { Observable, forkJoin, merge, concat } from 'rxjs';
import { map } from 'rxjs/operators';


const path = require('path');
const { Tail } = require('tail');
const moment = require('moment');
const colors = require('colors');
const encoding = require('encoding');

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

    const allMessage = encoding.convert(lineData.message, "UTF8","GBK").toString()

    let messages = allMessage.split('\n').filter((m: string) => m !== '');
    return messages.map((message: string) => {
        message = message.split('\n[').join('[')

        if(message.split('[').length < 4) {
            const updateTime = moment(lineData.timestamp).format('MM/DD HH:mm:ss.000');
            const type = 'error'.includes(lineData.type) ? 'error' : lineData.type;
            message = `[${updateTime}] [ ${type}  ] ${message.trim()}`
        }

        const msgList = message.split(']');
        const updateTime = msgList[0].slice(1)
        const updateTimeResolve = dealUpdateTime(updateTime)
        const typeResolve = `[${msgList[1].trim().slice(1).trim()}]`;
        
        let lastInfo = ''
        if(msgList.length >= 4) {
            lastInfo = msgList.slice(4).join(']')
        } else {
            lastInfo = msgList.slice(2).join(']')
        }

        const messageResolve = `[${updateTimeResolve}]${typeResolve}${lastInfo}`

        message = messageResolve
            .replace(/\[info\]/g, `[ ${colors[logColor.info]('info')}    ] `)
            .replace(/\[out\]/g,    `[ out     ] `)
            .replace(/\[trace\]/g, `[ trace   ] `)
            .replace(/\[error\]/g, `[ ${colors[logColor.error]('error')}   ] `)
            .replace(/\[warning\]/g, `[ ${colors[logColor.warning]('warning')} ] `)
            .replace(/\[debug\]/g, `[ ${colors[logColor.debug]('debug')}   ] `)
            .replace(/\[critical\]/g, `[ ${colors[logColor.critical]('critical')}] `)
    
        if(message.includes('critical')) message = `${colors[logColor.critical](message)}`
        
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
        .then((logList: NumList) => observer.next(logList))
        .catch(() => observer.next({ list: [] }))
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
    logWather && (logWather.unwatch());
    return new Observable(observer => {
        const logPath = path.join(LOG_DIR, `${processId}.log`);
        addFileSync('', logPath, 'file');
        const watcher = new Tail(logPath, {
            useWatchFile: true
        });

        logWather = watcher;
        
        watcher.watch();
        watcher.on('line', (line: string) => {
            const logList: any = dealLogMessage(line, processId);
            logList.kfForEach((l: any) => observer.next(l.message || ''))
        })
        watcher.on('error', () => watcher.unwatch())
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

// // =============================== logs end ==================================================
