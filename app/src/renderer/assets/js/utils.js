import readline from 'readline';
const path = require("path");
const fs = require('fs-extra');
const moment = require('moment');

//因为accountid都是source_accountID,需要截取掉柜台名称
String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
}

String.prototype.parseSourceAccountId = function(){
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
export const deepClone = (obj) => {
    if(!obj) return obj
    return JSON.parse(JSON.stringify(obj))
}
 

//保留几位数据
//(数据，保留位数，结果乘几个10，类型)
export const toDecimal = (num, digit = 2, multiply = 0, type = 'round') => {
    if(isNaN(parseFloat(num))) return ''; //如果为转换后为NaN,返回空
    //如果存在科学计数法的数据则返回不做处理
    if(num.toString().indexOf('e') != -1) return num.toExponential(2)

    const multiplyNum = Math.pow(10, multiply);
    let floatNum = parseFloat(num);
    const digitNum = Math.pow(10, digit);

    floatNum = Math[type](floatNum * digitNum)/(digitNum/multiplyNum);

    return floatNum;
}

/**
 * 防抖函数
 * @param {Function} fn 被执行函数
 * @param {number} interval 执行频率，默认为300
 */
export const debounce = (fn, interval=300) => {
    let timeout = null;
    return function() {
        clearTimeout(timeout);
        timeout = setTimeout(() => {
            if(!timeout) return;
            fn.apply(this, arguments);
        }, interval);
    }
}


export const throttleInsert = (interval=300, type='push') => {
    let streamList = [];
    let timer = null;
    return function(data){
        return new Promise((resolve, reject) => {
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
                clearTimeout(timer)
                timer = null;
            }, interval)
        })
    }
}

export const throttle = (fn, interval=300) => {
    let timer = null;
    return function(){
        if(timer !== null) return 
        timer = setTimeout(() => {
            fn.apply(this, arguments)
            clearTimeout(timer)
            timer = null;
        }, interval)
    }
}


/**
 * 新建窗口
 * @param  {string} htmlPath
 */
export const openWin = (htmlPath, BrowserWindow) => {
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
    // // Open the DevTools.
    // win.webContents.openDevTools()
}

/**
 * 启动任务，利用electron多进程
 * @param  {} taskPath
 */
export const buildTask = (taskPath, curWin, BrowserWindow) => {
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
export const ifProcessRunning = (processId, processStatus) => {
    if(!processStatus) return false
    return processStatus[processId] === 'online'
}

/**
 * 加法
 * @param  {Array} list
 */
export const sum = (list) => {
    return list.reduce((accumlator, a) => (accumlator + +a))
}



export const dealLogMessage = (line, searchKeyword) => {
    let lineData;
    try{
        lineData = JSON.parse(line);
    }catch(err){
        console.error(err)
        return false;
    }
    const message = lineData.message;
    //message 提取 ‘\n’ 再循环
    return message.split('\n[').map((m,i) => {
        if(!m.length) return false;
        if(i > 0) m = '[' + m;
        const messageList = m.split(']')
        const len = messageList.length;
        let messageData = {}
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
function buildListByLineNum(num){
    function ListByNum(n){
        this.list = [];
        this.len = 0;
        this.num = n;
    }
    ListByNum.prototype.insert = function(item){
        const t = this;
        if(t.len >= t.num) t.list.shift();
        else t.len++
        t.list.push(item)
    }
    return new ListByNum(num)
}

/**
 * 通过文件获取log
 * @param  {path} logPath
 * @param  {string} searchKeyword
 */
export const getLog = (logPath, searchKeyword) => {
    const numList = buildListByLineNum(201);    
    let logId = 0;            
    return new Promise((resolve, reject) => {
        fs.stat(logPath, (err, res) => {
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
                messageData.forEach(msg => {
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

export const buildDateRange = (dateRange, tradingDay, addTime=0) => {
    dateRange = dateRange || [];
    const momentDay = tradingDay ? moment(tradingDay) : moment();
    //获取当天是日期范围
    const startDate = Math.max((moment(momentDay.format('YYYY-MM-DD')).valueOf()) * 1000000, addTime)
    const endDate = (moment(momentDay.add(1,'d').format('YYYY-MM-DD')).valueOf()) * 1000000
    //日期控件选出的日期都是0点的，需要加上一天才能将最后一天包含在内
    const dateRange0 = Math.max(moment(dateRange ? dateRange[0] : undefined).valueOf() * 1000000, addTime);
    const dateRange1 = moment(dateRange ? dateRange[1] : undefined).add(1,'d').valueOf() * 1000000;
    return dateRange.length ? [dateRange0, dateRange1] : [startDate, endDate];
}