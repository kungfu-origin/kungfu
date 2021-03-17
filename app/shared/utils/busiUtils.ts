import fse from 'fs-extra';
import readline from 'readline';
import { EXTENSION_DIR } from '__gConfig/pathConfig';
import { listDir } from '__gUtils/fileUtils';

const path = require("path");

interface LogLineData {
    message: string;
    type: string;
    timestamp: string;
    [propName: string]: any;
}

interface LogMessageData {
    updateTime: string;
    type: string;
    pid: string;
    action: string;
    message: string;
}

interface SourceAccountId {
    source: string,
    id: string
}

interface SourceConfig {
    "name": string,
    "type": string,
    "key": string,
    "config": {}
}

interface DisplayConfig {
    "type": string;
    "align": string;
    "label": string
    "prop": string
    "width": string
}

const KUNGFU_KEY_IN_PACKAGEJSON = 'kungfuConfig'


declare global {
    interface String {
        toAccountId(): string;
        parseSourceAccountId(): SourceAccountId;
        toSourceName(): string;
    }

    interface Array<T> {
        removeRepeat(): any;
        kfForEach(cb: Function): any
    }
}

export {}

//因为accountid都是source_accountID,需要截取掉柜台名称
String.prototype.toAccountId = function(): string{
    if(this.indexOf('_') === -1) return this.toString();
    return this.split('_').slice(1).join('_')
}

String.prototype.toSourceName = function(): string {
    if(this.indexOf('_') === -1) return this.toString();
    return this.split('_')[0];
}


String.prototype.parseSourceAccountId = function(): SourceAccountId {
    const parseList = this.toString().split('_');
    //没有 "_"
    if(parseList.length !== 2) {
        throw new Error(`${this} accountId format is wrong！`)
    } else {
        return {
            source: parseList[0],
            id: parseList[1] 
        }
    }
}

Array.prototype.removeRepeat = function (): any {
    return Array.from(new Set(this))
}

Array.prototype.kfForEach = function (cb: Function): any {
    if (!cb) return;
    const t = this;
    let i = 0, len = t.length;
    while (i < len) {
        cb.call(t, t[i], i)
        i++
    }
}

export const delayMiliSeconds = (miliSeconds: number): Promise<void> => {
    return new Promise(resolve => {
        let timer = setTimeout(() => {
            resolve()
            clearTimeout(timer)
        }, miliSeconds)
    })
}

//深度克隆obj
export const deepClone = <T>(obj: T): T => {
    if(!obj) return obj
    return JSON.parse(JSON.stringify(obj))
}
 
//保留几位数据
//(数据，保留位数，结果乘几个10，类型)
export const toDecimal = (num = 0, digit = 2, multiply = 0, type = 'round'): string => {
    if (num === null) num = 0;
    if (isNaN(num)) return ''; //如果为转换后为NaN,返回空
    //如果存在科学计数法的数据则返回不做处理
    if (`${num}`.includes('e')) return new Number(num).toExponential(2)
    if (multiply === 0) return Number(num).toFixed(digit)

    const multiplyNum: number = Math.pow(10, multiply);
    let floatNum = parseFloat(num.toString());
    const digitNum: number = Math.pow(10, digit);
    // @ts-ignore
    const mathFunc = Math[type]
    floatNum = mathFunc(floatNum * digitNum) / (digitNum / multiplyNum);
    // const fixedNum: number = pa
    return new Number(floatNum).toFixed(digit)
}

/**
 * 防抖函数
 * @param {Function} fn 被执行函数
 * @param {number} interval 执行频率，默认为300
 */
export const debounce = (fn: Function, interval = 300): Function => {
    let timeout: NodeJS.Timer | null;
    return function() {
        //@ts-ignore
        const t: any = this;
        const args: any = arguments;
        timeout && clearTimeout(timeout);
        timeout = null;
        timeout = setTimeout(() => {
            if(!timeout) return;
            fn.apply(t, args);
            timeout && clearTimeout(timeout);
            timeout = null;
        }, interval);
    }
}

export const throttle = (fn: Function, interval = 300): Function => {
    let timer: NodeJS.Timer | null;
    return function(){
        if(timer) return 
        //@ts-ignore
        const t: any = this;
        const args: any = arguments;
        timer = setTimeout(() => {
            fn.apply(t, args);
            timer && clearTimeout(timer)
            timer = null
        }, interval)
    }
}

export const throttleInsert = (interval = 300, type = 'push'): Function => {
    let streamList: any = [];
    let timer: NodeJS.Timer | null;
    return (data: any) => {
        return new Promise((resolve) => {
            if(type === 'push'){
                if(data instanceof Array) streamList = [...streamList, ...data]
                else if(data) streamList.push(data)
            }else if(type === 'unshift'){
                if(data instanceof Array) streamList = [...data, ...streamList]
                else if(data) streamList.unshift(data)
            }
            if(timer) {
                resolve(false)
                return;
            }
            timer = setTimeout(() => {
                resolve(streamList)
                streamList = []
                timer && clearTimeout(timer)
                timer = null;
            }, interval)
        })
    }
}

/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openVueWin = (htmlPath: string, routerPath: string, electronRemote: any, windowConfig: {}) => {
    const BrowserWindow: any = electronRemote.BrowserWindow;
    const currentWindow: any = electronRemote.getCurrentWindow();
    
    let x: Number,y: Number;

    if (currentWindow) { //如果上一步中有活动窗口，则根据当前活动窗口的右下方设置下一个窗口的坐标
        const [ currentWindowX, currentWindowY ] = currentWindow.getPosition();
        x = currentWindowX + 10;
        y = currentWindowY + 10;
    }

    const modalPath = process.env.NODE_ENV !== 'production'
    ? `http://localhost:9090/${htmlPath}.html#${routerPath}`
    : `file://${__dirname}/${htmlPath}.html#${routerPath}`
    
    return new Promise(( resolve, reject ) => {
        let win = new BrowserWindow({
            x,
            y,
            width: 1080, 
            height: 766,
            backgroundColor: '#161B2E',
            parent: currentWindow,
            webPreferences: {
                nodeIntegration: true
            },
            ...windowConfig
        });
       
        win.webContents.loadURL(modalPath)
        win.webContents.on('did-finish-load', () => {
            if(!currentWindow || Object.keys(currentWindow).length == 0 ) {
                reject(new Error('当前页面没有聚焦！'))
                return;
            }
            resolve(win)
        })

        let winId = win.id;
        window && window.ELEC_WIN_MAP && window.ELEC_WIN_MAP.add(winId, win)
        win.on('close', () => { 
            window && window.ELEC_WIN_MAP && window.ELEC_WIN_MAP.delete(winId)
            win = null;
        })
    })
}

/**
 * 启动任务，利用electron多进程
 * @param  {} taskPath
 */
export const buildTask = (taskPath: string, electronRemote: any, debugOptions = { width: 0, height: 0, show: false }) => {
    
    const taskPathResolved = `file://${path.join(__resources, 'tasks', taskPath + '.html')}`;
    const BrowserWindow: any = electronRemote.BrowserWindow;
    const currentWindow: any = electronRemote.getCurrentWindow();
    
    return new Promise(( resolve, reject ) => {
        let win = new BrowserWindow({
            parent: currentWindow,
            webPreferences: {
                nodeIntegration: true
            },
            ...debugOptions,
		    backgroundColor: '#161B2E',
        })

        win.webContents.loadURL(taskPathResolved)
        win.webContents.on('did-finish-load', () => { 
            if(!currentWindow || Object.keys(currentWindow).length == 0 ) {
                reject(new Error('当前页面没有聚焦！'))
                return;
            }
            const curWinId = currentWindow.id;
            resolve({ win, curWinId })
        })

        let winId = win.id;
        window && window.ELEC_WIN_MAP && window.ELEC_WIN_MAP.add(winId, win)
        win.on('close', () => { 
            window && window.ELEC_WIN_MAP && window.ELEC_WIN_MAP.delete(winId)
            win = null;
            
        })
    })
}


/**
 * test if process is running
 * @param  {String} processId
 * @param  {Object} processStatus
 */
export const ifProcessRunning = (processId: string, processStatus: any): boolean => {
    if(!processStatus) return false
    return processStatus[processId] === 'online' || processStatus[processId] === 'stopping'
}

/**
 * 加法
 * @param  {Array} list
 */
export const sum = (list: number[]): number => {
    if (!list.length) return 0
    return list.reduce((accumlator, a) => (accumlator + +a))
}



export const dealLogMessage = (line: string, searchKeyword?: string):any => {
    let lineData: LogLineData;

    try{
        lineData = JSON.parse(line);
    }catch(err){
        console.error(err)
        return false;
    }

    const message = lineData.message;

    //message 提取 ‘\n’ 再循环
    return message.split('\n[').map((m: string, i: number) => {
        if(!m.length) return false;
        if(i > 0) m = '[' + m;
        
        const messageList = m.split(']')
        const len = messageList.length;
        let messageData: LogMessageData;
        
        switch (len) {
            case 5:
                messageData = {
                    updateTime: messageList[0].trim().slice(1).trim(),
                    type: messageList[1].trim().slice(1).trim(),
                    pid: messageList[2].slice(messageList[2].indexOf('pid/tid') + 7).trim(),
                    action: messageList[3].trim().slice(1).trim(),
                    message: messageList[4].trim()
                }
                break;
            case 4:
                messageData = {
                    updateTime: lineData.timestamp,
                    type: '',
                    pid: '',
                    action: '',
                    message: messageList.join(']')
                }
                break;
            default:
                if(len < 4) {
                    const type = lineData.type === 'err' ? 'error' 
                        : lineData.type === 'out' ? 'info' : lineData.type;
                    messageData = {
                        updateTime: lineData.timestamp,
                        type,
                        pid: '',
                        action: '',
                        message: messageList.slice(0).join(']').trim()
                    }
                } else {
                    messageData = {
                        updateTime: messageList[0].trim().slice(1).trim(),
                        type: messageList[1].trim().slice(1).trim(),
                        pid: messageList[2].slice(messageList[2].indexOf('pid/tid') + 7).trim(),
                        action: messageList[3].trim().slice(1).trim(),
                        message: messageList.slice(4).join(']').trim()
                    }
                }
        }

        if(searchKeyword && messageData.message.indexOf(searchKeyword) === -1 ){
            if(messageData.type.indexOf(searchKeyword) === -1){
                return false;
            }
        }

        return messageData
    })
}

export const getExtensions = async (extDir: string): Promise<any> => {
    const files = await listDir(extDir);
    const filesResolved = (files || []).map((fp: string) => path.join(extDir, fp))
    const statFiles = await Promise.all(filesResolved.map((fp: string) => fse.stat(fp)))
    
    const afterFilterFiles = filesResolved.filter((fp: string, index: number) => {
        const statData: any = statFiles[index];
        return statData.isDirectory()
    })

    const childFilesList = await Promise.all(afterFilterFiles.map((fp: string) => listDir(fp)))

    return afterFilterFiles.filter((fp: string, index: number) => {
        const childFiles: any = childFilesList[index];
        if(childFiles.includes('package.json')) return true;
        else return false
    })
}

export const getExtensionPaths = (extDir: string): Promise<any> => {
    return getExtensions(extDir).then((filePaths: string[]): string[] => {
        return filePaths.map((fp: string): string => path.resolve(path.join(fp, 'package.json')))
    })
}

export const getExtensionConfigs = async (extDir: string): Promise<any> => {
    try {
        const packageJSONPaths: string[] = await getExtensionPaths(extDir)
        const packageJsons = await Promise.all(packageJSONPaths.map((p: string) => fse.readJson(p)))
        return packageJsons
            .map((p: any, index: number) => {
                const kungfuConfig = p[KUNGFU_KEY_IN_PACKAGEJSON];
                if(kungfuConfig) {
                    const type: string = kungfuConfig.type;
                    const uniKey: string | Array<string>= kungfuConfig.uniKey;
                    const config: SourceConfig = kungfuConfig.config
                    const displayConfig: DisplayConfig = kungfuConfig.displayConfig || {} 
                    return  {
                        type,
                        config,
                        uniKey,
                        key: kungfuConfig.key,
                        name: kungfuConfig.name,
                        packageJSONPath: packageJSONPaths[index],
                        displayConfig
                    }
                }
            })
            .filter(config => !!config)

    } catch (err) {
        console.error(err)
    }
}

export const getSourceList = () => getExtensionConfigs(EXTENSION_DIR)
    .then(extList => {
        const sourceList = extList
        .filter((e: any) => e.type === "source")
        .map((e: any) => (e.config || {}).name || "")
        .filter((e: any) => !!e)
        .map((e: any) => ({ name: e, value: e }));
        return sourceList
    })


export const setTimerPromiseTask = (fn: Function, interval = 500) => {
    var taskTimer: NodeJS.Timer | null = null;
    var clear = false;
    function timerPromiseTask (fn: Function, interval = 500) {
        if(taskTimer) clearTimeout(taskTimer)
        fn()
        .finally(() => {
            if (clear) {
                if(taskTimer) clearTimeout(taskTimer);
                return;
            } 
            taskTimer = setTimeout(() => {
                timerPromiseTask(fn, interval)
            }, interval)
        })
    }
    timerPromiseTask(fn, interval)
    return {
        clearLoop: function () {
            clear = true;
            if(taskTimer) clearTimeout(taskTimer);
        }
    }
} 

export const loopToRunProcess = async (promiseFunc: Array<Function>, interval = 1000) => {
    let i = 0, len = promiseFunc.length;
    let resList = [];
    for (i = 0; i < len; i++) {
        const pFunc = promiseFunc[i];
        try {
            const res = await pFunc();
            resList.push(res);
        } catch (err) {
            resList.push(err)
        }
        
        await(delayMiliSeconds(interval))
    }
    return resList
}


export const findTargetFromArray = ( list: any[], targetKey: string, targetValue: string | number ) => {
    const targetList = list.filter(item => item[targetKey] === targetValue )
    if (targetList) {
        return targetList[0]
    }
    return null
}

export const getIndexFromTargetTickers = (tickerList: TickerInTickerSet[], ticker: TickerInTickerSet) => {
    return tickerList.findIndex(item => {
        if (item.exchangeId === ticker.exchangeId) {
            if (item.instrumentId === ticker.instrumentId) {
                return true;
            }
        }
        return false;
    })
}  

export const ensureNum = (num: number | bigint | string) => {
    num = +(num.toString());
    if (Number.isNaN(num)) return 0
    if (!Number.isFinite(num)) return 0
    if (num === 1.7976931348623157e+308) return 0
    return +num
}

export const getDefaultRenderCellClass = (prop: string, item: any) => {
    switch (prop) {
        case 'mode':
            if (item.mode === '模拟') return 'green';
            else if (item.mode === '实盘') return 'blue';
            break;
        case 'side':
            if (item.side === '买') return 'red';
            else if (item.side === '卖') return 'green';
            break;
        case 'offset':
            if (item.offset === '开仓') return 'red';
            else if (item.offset === '平仓') return 'green';
            break;
        case 'statusName':
            if (+item.status === 4) return 'red';
            else if ([3, 5, 6].indexOf(+item.status) !== -1) return 'green';
            else return 'gray';
        case 'direction':
            if (item.direction === '多') return 'red';
            else if (item.direction === '空') return 'green';
            break;
        case 'realizedPnl':
            if (+item.realizedPnl > 0) return 'red';
            else if (+item.realizedPnl < 0) return 'green';
            break;
        case 'unRealizedPnl':
            if (+item.unRealizedPnl > 0) return 'red';
            else if (+item.unRealizedPnl < 0) return 'green';
            break;
        case 'lastPrice':
            if (+item.lastPrice > +item.avgPrice) {
                return item.direction === '多' ? 'red' : 'green';
            } else if (+item.lastPrice < +item.avgPrice) {
                return item.direction === '多' ? 'green' : 'red';
            }
            break;
        case 'clientId':
        case 'accountId':
            if ((item.clientId || '').toLowerCase().includes('手动')) return 'blue';
            break;
    }

    return ''
}


export function checkAllMdProcess (tickers: TickerInTickerSet[], processStatus: StringToStringObject) {
    let mds: any = {};
    tickers.forEach(item => {
        mds[item.source] = true;
    })

    const unrunningSources = Object.keys(mds || {}).filter(source => {
        const processId = `md_${source}`;
        if (!ifProcessRunning(processId, processStatus)) {
            return true
        } else {
            return false
        }
    })

    if (unrunningSources.length) {
        // @ts-ignore
        this.$message.warning(`${unrunningSources.join(', ')} 行情进行未开启!`)
        return false
    } else {
        return true
    }
}