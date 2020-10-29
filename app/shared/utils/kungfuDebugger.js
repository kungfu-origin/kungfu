const pm2 = require('pm2');
const path = require('path');
const fs = require('fs-extra');
const electron = require('electron');

const category = { MD: 0, TD: 1, STRATEGY: 2, SYSTEM: 3}

const mode = {LIVE: 0, DATA: 1, REPLAY:2, BACKTEST: 3}

const InstrumentType = {
    Unknown: 0,     //未知
    Stock: 1,       //普通股票
    Future: 2,      //期货
    Bond: 3,        //债券
    StockOption: 4, //股票期权
    Fund: 5,        //基金
    TechStock: 6,   //科创板股票
    Index: 7,       //指数
    Repo:8         //回购
}

const Side = {
    Buy: 0,//买入
    Sell: 1,//卖出
    Lock: 2,//锁仓
    Unlock: 3,//解锁
    Exec: 4,//行权
    Drop: 5,//放弃行权
    Purchase: 6,//申购
    Redemption: 7,//赎回
    Split: 8,//拆分
    Merge:9 ,//合并
    MarginTrade: 10,//融资买入
    ShortSell: 11,//融券卖出
    RepayMargin:12,//卖券还款
    RepayStock:13,//买券还券
    CashRepayMargin:14,//现金还款
    StockRepayStock:15,//现券还券
    SurplusStockTransfer:16,//余券划转
    GuaranteeStockTransferIn:17,//担保品转入
    GuaranteeStockTransferOut:18//担保品转出
}

const Offset = {Open: 0, Close:1, CloseToday:2, CloseYesterday:3 }

const ExchangeId = {SHFE: 'SHFE', DCE: 'DCE', CZCE: 'CZCE', SSE: 'SSE'}

function sleep(ms) {
      return new Promise(resolve => setTimeout(resolve, ms));
}

function objectFlip(obj) {
  const ret = {};
  Object.keys(obj).forEach(key => {
    ret[obj[key]] = key;
  });
  return ret;
}

function getCategoryName(c) {
    return (objectFlip(category)[c] || '').toLowerCase()
}

function getModeName(m) {
    return (objectFlip(mode)[m] || '').toLowerCase()
}

const kfDebugger = new class {
    constructor() {
        console.log(watcher)
        this.kfHome =  path.join(electron.remote.app.getPath('userData'), 'app');
        this.futureTickers = fs.readJsonSync(path.join(process.resourcesPath, 'kungfu-resources', 'config', 'futureTickers.json'), { throws: false })
        this.stockTickers = {}
        const stockArray = fs.readJsonSync(path.join(process.resourcesPath, 'kungfu-resources', 'config', 'stockTickers.json'), { throws: false })

    }

    removeConfig(location) { return watcher.config.removeConfig(location)}

    getAllConfig() { return watcher.config.getAllConfig() }

    getConfig(location) { return watcher.config.getConfig(location) }

    makeLocation(category, mode, group, name) {
        if (typeof  category === "number") {
            category = getCategoryName(category)
        }
        if (typeof mode === 'number') {
            mode = getModeName(mode)
        }
        return {'category': category, 'mode': mode, 'group': group, 'name': name}
    }

    makeMdLocation(source) {
        return this.makeLocation(category.MD, mode.LIVE, source, source)
    }

    makeTdLocation(source, name) {
        return this.makeLocation(category.TD, mode.LIVE, source, name)
    }

    makeStrategyLocation(name) {
        return this.makeLocation(category.STRATEGY, mode.LIVE, 'default', name)
    }

    setConfig(location, value) {
        var kfConfig = kungfu.longfist.Config()
        kfConfig.category = typeof location.category === "number" ? getCategoryName(location.category) : location.category
        kfConfig.mode = typeof location.mode === "number" ? getModeName(location.mode) : location.mode
        kfConfig.group = location.group
        kfConfig.name = location.name
        kfConfig.value = JSON.stringify(value)
        return watcher.config.setConfig(kfConfig)
    }

    setTargetAccountLocation(accountLocation) {
        this.targetAccountLocation = accountLocation
    }

    setTargetStrategyLocation(strategyLocation) {
        this.targetStrategyLocation = strategyLocation
    }

    makeOrder(instrumentId, instrumentType, exchangeId, limitPrice, volume, side, offset) {
        var newOrder = kungfu.longfist.OrderInput()
        newOrder.instrument_id = instrumentId
        newOrder.instrument_type = instrumentType
        newOrder.exchange_id = exchangeId
        newOrder.limit_price = limitPrice
        newOrder.volume = volume
        newOrder.side = side
        newOrder.offset = offset
        return newOrder
    }

    async issueOrder(order) {
        if (! this.targetAccountLocation) {
            return false;
        }
        if (!watcher.isReadyToInteract(this.targetAccountLocation)) {
                await sleep(5000);
        }
        console.log(watcher.appStates)
        if (!watcher.isReadyToInteract(this.targetAccountLocation)) {
            console.error(`tareget account ${this.targetAccountLocation.name}@${this.targetAccountLocation.group} is not ready to interact`)
            return false;
        }
        if (! this.targetStrategyLocation) {
            const result = watcher.issueOrder(order, this.targetAccountLocation)
            console.log(`issue order to ${this.targetAccountLocation.name}@${this.targetAccountLocation.group}, result is ${result}`)
            return result
        }
        else {
            console.log(`issue order from ${this.targetStrategyLocation.name} to ${this.targetAccountLocation.name}@${this.targetAccountLocation.group}`)
            return watcher.issueOrder(order, this.targetAccountLocation, this.targetStrategyLocation)
        }
    }

    buy(ticker, price, volume) {
        if (! ticker in this.stockTickers) {
            return false;
        }
        else {
            const order = this.makeOrder(ticker, InstrumentType.Stock, this.stockTickers[ticker].exchangeId.toUpperCase(), price, volume, Side.Buy, Offset.Open)
            return this.issueOrder(order)
        }
    }

    sell(ticker, price, volume) {
        if (! ticker in this.stockTickers) {
            return false;
        }
        else {
            const order = this.makeOrder(ticker, InstrumentType.Stock, this.stockTickers[ticker].exchangeId.toUpperCase(), price, volume, Side.Sell, Offset.Open)
            console.log(order)
            return this.issueOrder(order)
        }
    }

    buy_open() {

    }

    buy_close() {

    }

    sell_open() {

    }

    sell_close() {

    }

    startProcess(location) {
        var options = {
            "script": `kfc`,
            "cwd": path.join(process.resourcesPath, 'kfc'),
            "logType": "json",
            "mergeLogs": true,
            "logDateFormat": "YYYY-MM-DD HH:mm:ss",
            "autorestart": false,
            "maxRestarts": 1,
            "watch": false,
            "force": true,
            "execMode": "fork",
            "env": {
                "KF_HOME": this.kfHome,
            },
            "killTimeout": 16000
        }
        var name = ''
        if (location.category === 'td' || location.category === category.TD) {
            name = `td_${location.group}_${location.name}`
            const logPath = path.join(this.kfHome, 'log', `${name}.log`)
            options = {
                ...options,
                'name': name,
                "output": logPath,
                "error": logPath,
                'args':  `td -s ${location.group} -a ${location.name}`
            }

        } else if (location.category === 'md' || location.category === category.MD) {
            name = 'md_' + location.group
            const logPath = path.join(this.kfHome, 'log', `${name}.log`)
            options = {
                ...options,
                'name': name,
                "output": logPath,
                "error": logPath,
                'args': `md -s ${location.group}`,
            }
        } else if (location.category === 'strategy' || location.category == category.STRATEGY) {
            const config = JSON.parse(this.getConfig(location).value)
            const logPath = path.join(this.kfHome, 'log', `${config.strategy_id}.log`)
            name = config.strategy_id
            options = {
                ...options,
                'name': config.strategy_id,
                "output": logPath,
                "error": logPath,
                'args': `strategy -n ${config.strategy_id} -p ${config.strategy_path}`,
            }
        }
        console.log('pm2 describe ', name)
        console.log(options)
        pm2.describe(name, (err, processDescriptionList) => {
             if (processDescriptionList  === undefined || processDescriptionList.length === 0) {
                 pm2.start(options, (err, proc) => { console.log('started')})
             } else if (processDescriptionList[0].pid === undefined) {
                 pm2.restart(name, (err, proc) => {
                     console.log(`restart ${name}`)
                 })
             }
        })
    }

    execLocalFile(filePath) {
       return fs.readFile(filePath, 'utf8', function (err, data) {
           eval(data)})
    }
}();

module.exports = {kfDebugger}