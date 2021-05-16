import fse from 'fs-extra';
import iconv from 'iconv-lite';
import jschardet from 'jschardet';
import path from 'path';
import readline from 'readline';

import { EXTENSION_DIR } from '__gConfig/pathConfig';
import { listDir } from '__gUtils/fileUtils';
import { allowShorted } from "kungfu-shared/config/tradingConfig";


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
        kfForEach(cb: Function): any,
        kfForEachAsync(cb: Function): any
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
    const len = t.length;
    let i = 0;
    
    while (i < len) {
        cb.call(t, t[i], i);
        i++;
    }
}

Array.prototype.kfForEachAsync = kfForEachAsync;

function setImmediateIter (list: Array<any>, i: number, len: number, cb: Function, fcb: Function) {
    if (i < len) {
        setImmediate(() => {
            cb(list[i], i)
            setImmediateIter(list, ++i, len, cb, fcb)
        })
    } else {
        fcb()
    } 
}

function kfForEachAsync (cb: Function) {
    //@ts-ignore
    const t = this;
    const len = t.length;
    return new Promise(resolve => {
        setImmediateIter(t, 0, len, cb, () => {
            resolve(true)
        })
    })
   
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
    if (num.toString() === "") return '';
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
        fn.apply(t, args);
        timeout = setTimeout(() => {
            timeout && clearTimeout(timeout);
            timeout = null;
        }, interval);
    }
}

export const throttle = (fn: Function, interval = 300): Function => {
    let timer: NodeJS.Timer | null;
    return function(){
        if (timer) return;
        //@ts-ignore
        const t: any = this;
        const args: any = arguments;
        fn.apply(t, args);
        timer = setTimeout(() => {
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
export const openPage = (taskPath: string, electronRemote: any, debugOptions = { width: 0, height: 0, show: false }) => {
    
    const taskPathResolved = `file://${path.join(__resources, 'pages', taskPath + '.html')}`;
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

export const getMemCpu = (processId: string, processStatusWithDetail: any, type: string) => {
    const processData = processStatusWithDetail[processId] || {};
    const monit = processData.monit || {};    
    if (type === 'cpu') {
        return monit.cpu !== undefined ? Number(monit.cpu).toFixed(1) + '%' : '--';
    } else if (type === 'memory') {
        return monit.memory !== undefined ? Number(monit.memory / 1000000).toFixed(0) + "M" : '--';
    } else {
        return '--'
    }
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
    if (!fse.existsSync(extDir)) return [];
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
                    const subType: string = kungfuConfig.subType;
                    const config: SourceConfig = kungfuConfig.config;
                    const displayMode: string = kungfuConfig.displayMode || '';
                    const displayConfig: DisplayConfig = kungfuConfig.displayConfig || {} 
                    return  {
                        type,
                        config,
                        uniKey,
                        subType, 
                        key: kungfuConfig.key,
                        name: kungfuConfig.name,
                        packageJSONPath: packageJSONPaths[index],
                        displayConfig,
                        displayMode
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
        case 'selected':
            if (item.selected === '✓') return 'green';
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
        case 'clientId':
        case 'accountId':
            if ((item.clientId || '').toLowerCase().includes('手动')) return 'yellow';
            if ((item.clientId || '').toLowerCase().includes('任务')) return 'blue';
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

export function decodeBuffer (name: string[]) {
    name = name.filter(n => !!n);
    const bufferFrom = Buffer.from(name);
    return isBufferGBK(bufferFrom) ? iconv.decode(bufferFrom, 'gbk') : iconv.decode(bufferFrom, 'utf8')
}

export function isBufferGBK (bufferFrom: Buffer) {
    return jschardet.detect(bufferFrom).encoding !== 'UTF-8'
}

export const resolveInstruments = (instruments: InstrumentOriginData[]) => {
    return (instruments || []).map(item => {
        const { instrument_id, product_id, exchange_id } = item;
        const instrumentName = decodeBuffer(product_id)
        return {
            instrument_id,
            instrument_name: instrumentName,
            exchange_id,
            id: `${instrument_id}${instrumentName}${exchange_id}`.toLowerCase()
        }

    })
}

export function getLog(logPath: string, searchKeyword: string, dealMessageFunc: Function){
    const numList = buildListByLineNum(1000000000);    
    let logId = 0;            
    return new Promise((resolve, reject) => {
        fse.stat(logPath, (err: Error) => {
            if(err){
                reject(err)
                return;
            }

            const lineReader = readline.createInterface({
                input: fse.createReadStream(logPath, {
                    start: 0
                })
            })

            lineReader.on('line', line => {
                const messageData = dealMessageFunc(line, searchKeyword)
                if(!messageData || !messageData.length) return;
                messageData.forEach((msg: any) => {
                    if(!msg) return;
                    logId++
                    numList.insert({
                        ...msg,
                        id: logId
                    })
                })
            })

            lineReader.on('close', () => {
                resolve(numList)
            })
        })
    })
}

//建立固定条数的list数据结构
function buildListByLineNum(num: number){
    function ListByNum(n: number){
        //@ts-ignore
        this.list = [];
        //@ts-ignore
        this.len = 0;
        //@ts-ignore
        this.num = n;
    }

    ListByNum.prototype.insert = function(item: any){
        if(this.len >= this.num) this.list.shift();
        else this.len++
        this.list.push(item)
    }

    //@ts-ignore
    return new ListByNum(num)
}

export function ensureLeaderData (data: any, key = '') {
    if (!key) {
        return data ? data.list() : []
    }

    return data ? data.sort(key) : []
}

const orderTradesFilterByDirection = (direction: number, offset: number, side: number, instrumentType: number) => {
    if (!allowShorted(+instrumentType)) {
        return true;
    }

    // long
    if (+direction === 0) {
        if (+offset === 0) {
            if (+side === 0) {
                return true
            }
        } else {
            if (+side === 1) {
                return true
            }
        }
    } else { //short
        if (+offset === 0) {
            if (+side === 1) {
                return true;
            }
        } else {
            if (+side === 0) {
                return true;
            }
        }
    } 

    return false;
}

export const originOrderTradesFilterByInstrumentIdDirection = (item: OrderOriginData | TradeOriginData, instrumentId: string, directionOrigin: number) => {
    if (!instrumentId.includes(item.instrument_id)) {
        return false;
    }

    const { offset, side, instrument_type } = item;
    return orderTradesFilterByDirection(directionOrigin, offset, side, instrument_type)
}

export const buildDictFromArray = (list: any[], key: string) => {
    let data: { [prop: string]: any } = {};
    const keys: string[] = key.split(",");
    list.kfForEach((item: any) => {
        const key: string = keys.map((k: string): string => (item[k] || "").toString()).join('_');
        data[key] = item;
    })
    return data;
}

export const addTwoItemByKeyForReduce = (item1: any, item2: any, key: string) => {
    return (+item1[key] || 0) + (+item2[key] || 0);
}

export const avgTwoItemByKeyForReduce = (item1: any, item2: any, key: string) => {
    return addTwoItemByKeyForReduce(item1, item2, key) / 2;
}

