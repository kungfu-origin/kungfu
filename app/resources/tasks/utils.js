const readline = require('readline');
const fs = require('fs-extra'); 

//清空文件内容
function clearFileContent(filePath) {
    filePath = path.normalize(filePath)
    return new Promise((resolve, reject) => {
        fs.outputFile(filePath, '', (err, data) => {
            if(err){
                reject(err);
                return;
            }
            resolve(data)
        })
    })
}
  
//建立固定条数的list数据结构
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

function dealMessage(line, searchKeyword){
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
                    updateTime: lineData.timestamp,
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

function getLog(logPath, searchKeyword, dealMessageFunc){
    const numList = buildListByLineNum(201);    
    let logId = 0;            
    return new Promise((resolve, reject) => {
        fs.stat(logPath, (err,stats) => {
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
                const messageData = dealMessageFunc(line, searchKeyword)
                if(!messageData || !messageData.length) return;
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

function debounce(fn, interval = 300) {
    let timeout = null
    return function() {
        //@ts-ignore
        const t = this;
        const args = arguments;
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

exports.getLog = getLog;
exports.dealMessage = dealMessage;
exports.debounce = debounce;
exports.clearFileContent = clearFileContent;