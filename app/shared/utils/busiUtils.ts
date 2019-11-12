import readline from 'readline';
import { offsetName, orderStatus, sideName, posDirection } from "__gConfig/tradingConfig";
import { EXTENSION_DIR } from '__gConfig/pathConfig';
import { listDir, statSync, readJsonSync } from '__gUtils/fileUtils';

const path = require("path");
const fs = require('fs-extra');
const moment = require('moment');

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

interface ExtensionJSON {
    type: string,
    config: SourceConfig
}

const KUNGFU_KEY_IN_PACKAGEJSON = 'kungfuConfig'


declare global {
    interface String {
        toAccountId(): string;
        parseSourceAccountId(): SourceAccountId;
    }

    interface Array<T> {
        removeRepeat(): any;
    }
}

export {}

//因为accountid都是source_accountID,需要截取掉柜台名称
String.prototype.toAccountId = function(): string{
    if(this.indexOf('_') === -1) return this.toString();
    return this.split('_').slice(1).join('_')
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

Array.prototype.removeRepeat = function(): any{
    return Array.from(new Set(this))
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
    if(num === null) num = 0;
    if(isNaN(parseFloat(num.toString()))) return ''; //如果为转换后为NaN,返回空
    //如果存在科学计数法的数据则返回不做处理
    if(num.toString().indexOf('e') != -1) return new Number(num).toExponential(2)
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


/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openWin = (htmlPath: string, BrowserWindow: any): void => {

    const modalPath = process.env.NODE_ENV !== 'production'
    ? `http://localhost:9090/#/${htmlPath}`
    : `file://${__dirname}/index.html#${htmlPath}`
    
    let win = new BrowserWindow({
        width: 1080, 
        height: 766,
        backgroundColor: '#161B2E',
        webPreferences: {
            nodeIntegration: true
        },
    });
    win.loadURL(modalPath)
    // win.webContents.openDevTools()
    win.show()
    win.on('close', () => win = null)
}

/**
 * 启动任务，利用electron多进程
 * @param  {} taskPath
 */
export const buildTask = (
    taskPath: string, 
    curWin: any, 
    BrowserWindow: any, 
    debugOptions = { 
        width: 0,
        height: 0,
        show: false
    }) => {
    const taskFullPath = `file://${path.join(__resources, 'tasks', taskPath + '.html')}`;
    return new Promise((resolve, reject) => {
        const win = new BrowserWindow({
            webPreferences: {
                nodeIntegration: true
            },
            ...debugOptions,
		    backgroundColor: '#161B2E',
        })

        win.webContents.loadURL(taskFullPath)
        win.webContents.on('did-finish-load', () => { 
            if(!curWin || Object.keys(curWin).length == 0 ) {
                reject(new Error('当前页面没有聚焦！'))
                return;
            }
            const curWinId = curWin.id;
            resolve({win, curWinId})
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
    return processStatus[processId] === 'online'
}

/**
 * 加法
 * @param  {Array} list
 */
export const sum = (list: number[]): number => {
    return list.reduce((accumlator, a) => (accumlator + +a))
}



export const dealLogMessage = (line: string, searchKeyword?: string):any => {
    let lineData: LogLineData;
    try{
        lineData = JSON.parse(line);
    }catch(err){
        // console.error(err)
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
        switch(len){
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
                if(len < 4){
                    const type = lineData.type === 'err' ? 'error' 
                        : lineData.type === 'out' ? 'info' : lineData.type;
                    messageData = {
                        updateTime: lineData.timestamp,
                        type,
                        pid: '',
                        action: '',
                        message: messageList.slice(0).join(']').trim()
                    }
                }else{
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

/**
 * 建立固定条数的list数据结构
 * @param  {number} num
 */

function buildListByLineNum(num: number): any {
    class ListByNum {
        list: any[];
        len: number;
        num: number;
        constructor(n: number) {
            this.list = [];
            this.len = 0;
            this.num = n;
        }
        insert(item: object): void {
            const t = this;
            if(t.len >= t.num) t.list.shift();
            else t.len++
            t.list.push(item)
        }
    }
    return new ListByNum(num)
}

/**
 * 通过文件获取log
 * @param  {path} logPath
 * @param  {string} searchKeyword
 */
export const getLog = (logPath: string, searchKeyword?: string, dealLogMessageMethod = dealLogMessage): Promise<any> => {
    const numList: NumList = buildListByLineNum(50);    
    let logId: number = 0;            
    return new Promise((resolve, reject) => {
        fs.stat(logPath, (err: Error, stats: any) => {
            if(err){
                reject(err)
                return;
            }

            const startSize = stats.size - 1000000 < 0 ? 0 : stats.size - 1000000;
            const lineReader = readline.createInterface({
                input: fs.createReadStream(logPath, {
                    start: startSize
                })
            })

            lineReader.on('line', line => {
                const messageData = dealLogMessageMethod(line, searchKeyword)
                if(!messageData) return;
                messageData.forEach((msg: LogMessageData): void => {
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

export const buildDateRange = (dateRange: string[], tradingDay?: string): Array<string|undefined> => {
    if(dateRange.length === 2) {
        return [moment(dateRange[0]).format('YYYYMMDD'), moment(dateRange[1]).format('YYYYMMDD')]
    } else if (tradingDay) {
        tradingDay = moment(tradingDay).format('YYYYMMDD')
        return [tradingDay, tradingDay]
    } else throw new Error('dateRange == [] and tradingDay undefined!')
}

// ========================== 交易数据处理 start ===========================

export const dealOrder = (item: OrderInputData): OrderData => {
    const updateTime = item.update_time || item.insert_time || 0;
    return Object.freeze({
        id: item.order_id.toString() + '_' + item.account_id.toString(),
        updateTime: moment(updateTime / 1000000).format("YYYY-MM-DD HH:mm:ss"),
        updateTimeNum: +updateTime,
        instrumentId: item.instrument_id,
        side: sideName[item.side] ? sideName[item.side] : '--',
        offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
        limitPrice: toDecimal(item.limit_price, 3) || '--',
        volumeTraded: item.volume_traded + "/" + (item.volume),
        statusName: orderStatus[item.status],
        status: item.status,
        clientId: item.client_id,
        accountId: item.account_id,
        orderId: item.order_id,
        exchangeId: item.exchange_id
    })
}

export const dealTrade = (item: TradeInputData): TradeData => {
    const updateTime = item.trade_time || item.update_time || 0;
    return {
        id: [(item.rowid || '').toString(), item.account_id.toString(), item.trade_id.toString(), updateTime.toString()].join('_'),
        updateTime: updateTime && moment(+updateTime / 1000000).format('YYYY-MM-DD HH:mm:ss'),
        updateTimeNum: +updateTime,
        instrumentId: item.instrument_id,
        side: sideName[item.side],
        offset: offsetName[item.offset],
        price: toDecimal(+item.price, 3),
        volume: toDecimal(+item.volume, 0),
        clientId: item.client_id,
        accountId: item.account_id
    }     
}

export const dealPos = (item: PosInputData): PosData => {
    //item.type :'0': 未知, '1': 股票, '2': 期货, '3': 债券
    const direction: string = posDirection[item.direction] || '--';
    return Object.freeze({
        id: item.instrument_id + direction,
        instrumentId: item.instrument_id,
        direction,
        yesterdayVolume: toDecimal(item.yesterday_volume, 0),
        todayVolume: toDecimal(item.volume - item.yesterday_volume, 0),
        totalVolume: toDecimal(item.volume, 0),
        avgPrice: toDecimal(item.avg_open_price || item.position_cost_price, 3) || '--',
        lastPrice: toDecimal(item.last_price, 3) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) + '' || '--'
    })
}

export const dealAsset = (item: AssetInputData): AssetData => {
    return {
        accountId: `${item.source_id}_${item.account_id}`,
        clientId: item.client_id,
        initialEquity: toDecimal(item.initial_equity) || '--',
        staticEquity: toDecimal(item.static_equity) || '--',
        dynamicEquity: toDecimal(item.dynamic_equity) || '--',
        realizedPnl: toDecimal(item.realized_pnl) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) || '--',
        avail: toDecimal(item.avail) || '--',
        marketValue: toDecimal(item.market_value) || '--',
        margin: toDecimal(item.margin) || '--'
    }
}




// ========================== 交易数据处理 end ===========================



export const getExtensions = (): Promise<any> => {
    return listDir(EXTENSION_DIR).then(async (files: string[]) => {
        const promises = files.map(fp => {
            fp = path.join(EXTENSION_DIR, fp);
            const stat: any = statSync(fp);
            let isDir: boolean;
            if(stat) isDir = stat.isDirectory();    
            else return false;
            if(isDir) {
                return listDir(fp).then((childFiles: string[]) => {
                    if(childFiles.indexOf('package.json') !== -1) return fp;
                    else return false;
                })
            } else {
                return false
            }
           
        })
        const fpList = await Promise.all(promises)
        return fpList.filter(f => !!f)
    })
}

export const getExtensionPaths = (): Promise<any> => {
    return getExtensions().then((filePaths: string[]): string[] => {
        return filePaths.map((fp: string): string => path.join(fp, 'package.json'))
    })
}

export const getExtensionConfigs = async (): Promise<any> => {
    try {
        const packageJSONPaths: string[] = await getExtensionPaths()
        return packageJSONPaths.map((p: string) => {
            const packageJSON: any = readJsonSync(p)
            const kungfuConfig: ExtensionJSON = packageJSON[KUNGFU_KEY_IN_PACKAGEJSON];
            if(kungfuConfig) {
                const type: string = kungfuConfig.type;
                const config: SourceConfig = kungfuConfig.config
                return  {
                    type,
                    config
                }
            }
        }).filter(config => !!config)
    } catch (err) {
        console.error(err)
    }
}

export const setTimerPromiseTask = (fn: Function, interval = 500) => {
    var taskTimer: NodeJS.Timer | null = null;
    function timerPromiseTask (fn: Function, interval = 500) {
        if(taskTimer) clearTimeout(taskTimer)
        fn()
        .finally(() => {
            taskTimer = setTimeout(() => {
                timerPromiseTask(fn, interval)
            }, interval)
        })
    }
    timerPromiseTask(fn, interval)
} 