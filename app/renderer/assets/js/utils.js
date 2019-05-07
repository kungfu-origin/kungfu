const BrowserWindow = require('electron').remote.BrowserWindow
const path = require("path");


//因为accountid都是source_accountID,需要截取掉柜台名称
String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
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
export const openWin = (htmlPath) => {
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
export const buildTask = (taskPath, preCurWin = BrowserWindow.getFocusedWindow()) => {
    const curWin = preCurWin || BrowserWindow.getFocusedWindow()
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

