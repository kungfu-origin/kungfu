import readline from 'readline';
import { offsetName, orderStatus, sideName, posDirection } from "__gConfig/tradingConfig";

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



declare global {
    interface String {
        toAccountId(): string;
        parseSourceAccountId(): SourceAccountId;
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

//深度克隆obj
export const deepClone = <T>(obj: T): T => {
    if(!obj) return obj
    return JSON.parse(JSON.stringify(obj))
}
 
//保留几位数据
//(数据，保留位数，结果乘几个10，类型)
export const toDecimal = (num = 0, digit = 2, multiply = 0, type = 'round'): number => {
    if(num === null) num = 0;
    if(isNaN(parseFloat(num.toString()))) return NaN; //如果为转换后为NaN,返回空
    //如果存在科学计数法的数据则返回不做处理
    if(num.toString().indexOf('e') != -1) return +num.toExponential(2)
    const multiplyNum: number = Math.pow(10, multiply);
    let floatNum = parseFloat(num.toString());
    const digitNum: number = Math.pow(10, digit);
    // @ts-ignore
    const mathFunc = Math[type]
    floatNum = mathFunc(floatNum * digitNum) / (digitNum / multiplyNum);
    return floatNum;
}

/**
 * 防抖函数
 * @param {Function} fn 被执行函数
 * @param {number} interval 执行频率，默认为300
 */
export const debounce = (fn: Function, interval = 300): Function => {
    let timeout: NodeJS.Timer;
    return function() {
        //@ts-ignore
        const t: any = this;
        const args: any = arguments;
        timeout && clearTimeout(timeout);
        timeout = setTimeout(() => {
            if(!timeout) return;
            fn.apply(t, args);
        }, interval);
    }
}

export const throttleInsert = (interval = 300, type = 'push'): Function => {
    let streamList: any = [];
    let timer: NodeJS.Timer;
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
            }, interval)
        })
    }
}

export const throttle = (fn: Function, interval=300): Function => {
    let timer: NodeJS.Timer;
    return function(){
        if(timer !== null) return 
        //@ts-ignore
        const t: any = this;
        const args: any = arguments;
        timer = setTimeout(() => {
            fn.apply(t, args);
            clearTimeout(timer)
        }, interval)
    }
}


/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openWin = (htmlPath: string, BrowserWindow: any): void => {
    const modalPath = process.env.NODE_ENV === 'development'
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
    win.show()
    win.on('close', () => win = null)
}

/**
 * 启动任务，利用electron多进程
 * @param  {} taskPath
 */
export const buildTask = (taskPath: string, curWin: any, BrowserWindow: any) => {
    const taskFullPath = `file://${path.join(__resources, 'tasks', taskPath + '.html')}`;
    return new Promise((resolve, reject) => {
        //debug
        const debugOptions = taskPath === '' ? {
            width: 800,
            height: 600,
            show: true,
        } : {}

        const win = new BrowserWindow({
            width: 0,
            height: 0,
            show: false,
            webPreferences: {
                nodeIntegration: true
            },
            ...debugOptions
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



export const dealLogMessage = (line: string, searchKeyword: string):any => {
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
                    updateTime: messageList[0].trim().slice(1).trim(),
                    type: messageList[1].trim().slice(1).trim(),
                    pid: '',
                    action: messageList[2].trim().slice(1).trim(),
                    message: messageList[3].trim(),
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
                        message
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
export const getLog = (logPath: string, searchKeyword: string): Promise<{}> => {
    const numList: any = buildListByLineNum(201);    
    let logId: number = 0;            
    return new Promise((resolve, reject) => {
        fs.stat(logPath, (err: Error) => {
            if(err){
                reject(err)
                return;
            }

            const lineReader = readline.createInterface({
                input: fs.createReadStream(logPath)
            })

            lineReader.on('line', line => {
                const messageData = dealLogMessage(line, searchKeyword)
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

export const buildDateRange = (dateRange: string[], tradingDay: string, addTime = 0): number[] => {
    dateRange = dateRange || [];
    const momentDay: any = tradingDay ? moment(tradingDay) : moment();
    //获取当天是日期范围
    const startDate: number = Math.max((moment(momentDay.format('YYYY-MM-DD')).valueOf()) * 1000000, addTime)
    const endDate: number = (moment(momentDay.add(1, 'd').format('YYYY-MM-DD')).valueOf()) * 1000000
    //日期控件选出的日期都是0点的，需要加上一天才能将最后一天包含在内
    const dateRange0: number = Math.max(moment(dateRange.length ? dateRange[0] : undefined).valueOf() * 1000000, addTime);
    const dateRange1: number = moment(dateRange.length ? dateRange[1] : undefined).add(1, 'd').valueOf() * 1000000;
    return dateRange.length ? [dateRange0, dateRange1] : [startDate, endDate];
}

// ========================== 交易数据处理 start ===========================
export const dealOrder = (item: any): OrderData => {
    return Object.freeze({
        id: item.order_id.toString() + '_' + item.account_id.toString(),
        insertTime: item.insert_time && moment(item.insert_time / 1000000).format("YYYY-MM-DD HH:mm:ss"),
        instrumentId: item.instrument_id,
        side: sideName[item.side] ? sideName[item.side] : '--',
        offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
        limitPrice: item.limit_price,
        volumeTraded: item.volume_traded + "/" + (item.volume),
        statusName: orderStatus[item.status],
        status: item.status,
        clientId: item.client_id,
        accountId: item.account_id,
        orderId: item.order_id,
        exchangeId: item.exchange_id
    })
}

export const dealPos = (item: any): PosData => {
    //item.type :'0': 未知, '1': 股票, '2': 期货, '3': 债券
    const direction: string = posDirection[item.direction] || '--';
    return Object.freeze({
        id: item.instrument_id + direction,
        instrumentId: item.instrument_id,
        direction,
        yesterdayVolume: toDecimal(item.yesterday_volume),
        todayVolume: toDecimal(item.volume - item.yesterday_volume),
        totalVolume: toDecimal(item.volume),
        openPrice: +toDecimal(item.open_price) || '--',
        lastPrice: +toDecimal(item.last_price) || '--',
        unRealizedPnl: toDecimal(item.unrealized_pnl) + '' || '--'
    })
}

export const dealTrade = (item: TradeInputData): TradeData => {
    return {
        id: item.account_id.toString() + '_' + item.trade_id.toString() + '_' + item.trade_time.toString(),
        tradeTime: item.trade_time && moment(item.trade_time/1000000).format('YYYY-MM-DD HH:mm:ss'),
        instrumentId: item.instrument_id,
        side: sideName[item.side],
        offset: offsetName[item.offset],
        price: item.price,
        volume: item.volume,
        clientId: item.client_id,
        accountId: item.account_id
    }     
}


// ========================== 交易数据处理 end ===========================

