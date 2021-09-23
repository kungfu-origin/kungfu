const readline = require('readline');
const fse = require('fs-extra'); 

//清空文件内容
function clearFileContent(filePath) {
    filePath = path.normalize(filePath)
    return fse.outputFile(filePath, '')
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

function getLog(logPath, searchKeyword, dealMessageFunc){
    const numList = buildListByLineNum(1000000000);    
    let logId = 0;            
    return new Promise((resolve, reject) => {
        fse.stat(logPath, (err) => {
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
exports.debounce = debounce;
exports.clearFileContent = clearFileContent;