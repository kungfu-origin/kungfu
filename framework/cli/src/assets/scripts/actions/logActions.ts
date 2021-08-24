import logColor from '@kungfu-trader/kungfu-uicc/config/logColorConfig';
import { buildProcessLogPath } from '@kungfu-trader/kungfu-uicc/config/pathConfig';

import { getLog, setTimerPromiseTask } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';


import { Subject } from 'rxjs';

const moment = require('moment');
const colors = require('colors');
export const logSubject: any = new Subject();


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

const dealLogMessage = (line: string) => {
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
        const updateTimeResolved = `[${dealUpdateTime(updateTime)}]`
        const typeResolve = `[${msgList[1].trim().slice(1).trim()}]`;
        
        let lastInfo = ''
        if(msgList.length >= 4) {
            lastInfo = msgList.slice(4).join(']')
        } else {
            lastInfo = msgList.slice(2).join(']')
        }

        const messageResolve = `${colors.cyan(updateTimeResolved)} ${typeResolve} ${lastInfo}`

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


export const getLogInterval = (pid: string) => {
    const logPath = buildProcessLogPath(pid)
    return setTimerPromiseTask(() => {
        return getLog(logPath, '', (line: string) => dealLogMessage(line))
        .then((logList: any) => logSubject.next(logList))
        .catch(() => logSubject.next({ list: [] }))
    }, 2000)
}

export const getLogSubject = () => {
    return logSubject
} 

export function splitStrByLength (str: string, len: number): string[] {

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


