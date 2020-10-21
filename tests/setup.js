const pm2 = require('pm2');
const path = require('path');
const fs = require("fs");
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

let kfDebugger = new class {
    constructor(name) {
        this.kfHome =  path.join(electron.remote.app.getPath('userData'), 'app');
        pm2.connect(true, function(err) {
            if (err) {
                console.error('failed to connect pm2 daemon ', err)
            } else {
                console.info('connected to pm2 daemon')
            }
        });
    }

    getAllConfig() {
        return watcher.config.getAllConfig()
    }

    setConfig(config) {
        return watcher.config.setConfig(config)
    }

    makeConfig(category, mode, group, value) {
        var kfConfig = kungfu.longfist.Config()
        kfConfig.category = category
        kfConfig.mode = mode
        kfConfig.group = group
        kfConfig.value = JSON.stringify(value)
        return kfConfig
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

    startProcess(configIndex) {
        var options = {
            "script": `kfc`,
            "cwd": path.join(process.resourcesPath, 'kfc'),
            "autorestart": false,
            "maxRestarts": 1,
            "watch": false,
            "force": true,
            "execMode": "fork",
            "env": {
                "KF_HOME": kfHome,
            },
        }
        if (config.category === 'td' || config.category === 0) {
            //TODO
        } else if (config.category == 'md' || config.category === 1) {
            options = {
                ...options,
                'name': 'md_' + config.group,
                'args': 'md -s ' + config.group,
            }
            pm2.start(options, function (err, apps) {
                console.log(err)})
        } else if (config.category == 'strategy' || config.category == 2) {
            //TODO
        }
    }

    issueOrder(order, tdConfigIdx) {
        const location = watcher.getLocation(tdConfigIdx)
        return watcher.issueOrder(order, location)
    }
    
    execLocalFile(filePath) {
       return fs.readFile(filePath, 'utf8', function (err, data) {
           eval(data)})
    }
}();
