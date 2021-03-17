
const isEnglish = process.env.LANG_ENV === 'en';

// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const OffsetName: NumberToStringObject = {
    0: isEnglish ? 'Open' : '开',
    1: isEnglish ? 'Close' : '平',
    2: isEnglish ? 'CloseToday' : '平今',
    3: isEnglish ? 'CloseYest' : '平昨'
}

export const SideName: NumberToStringObject = {
    0: isEnglish ? 'Buy'                                : '买',
    1: isEnglish ? 'Sell'                               : '卖',
    2: isEnglish ? 'Lock'                               : '锁仓',
    3: isEnglish ? 'Unlock'                             : '解锁',
    4: isEnglish ? 'Exec'                               : '行权',
    5: isEnglish ? 'Drop'                               : '放弃行权',
    6: isEnglish ? 'Purchase'                           : '申购',
    7: isEnglish ? 'Redemption'                         : '赎回',
    8: isEnglish ? 'Split'                              : '拆分',
    9: isEnglish ? 'Merge'                              : '合并',
    10:isEnglish ? 'MarginTrade'                        : '融资买入',
    11:isEnglish ? 'ShortSell'                          : '融券卖出',
    12:isEnglish ? 'RepayMargin'                        : '卖券还款',
    13:isEnglish ? 'RepayStock'                         : '买券还券',
    14:isEnglish ? 'CashRepayMargin'                    : '现金还款',
    15:isEnglish ? 'StockRepayStock'                    : '现券还券',
    16:isEnglish ? 'SurplusStockTransfer'               : '余券划转',
    17:isEnglish ? 'GuaranteeStockTransferIn'           : '担保品转入',
    18:isEnglish ? 'GuaranteeStockTransferOut'          : '担保品转出',
    
}


export const PosDirection: NumberToStringObject = {
    0: isEnglish ? 'Long' : '多',
    1: isEnglish ? 'Short' : '空' 
}

// Unknown = '0', // 未知
// Submitted = '1', //已提交 
// Pending = '2', // 等待
// Cancelled = '3', // 已撤销
// Error = '4', // 错误
// Filled = '5', //已成交
// PartialFilledNotActive = '6', //部分撤单
// PartialFilledActive = '7' //正在交易
// 0,3,4,5,6 已完成
export const OrderStatus: NumberToStringObject = {
    0: isEnglish ? 'Unknow' : '未知',
    1: isEnglish ? 'Submitted' : '已提交', // ing
    2: isEnglish ? 'Pending' : '等待中', // ing
    3: isEnglish ? 'Cancelled' : '已撤单', 
    4: isEnglish ? 'Error' : '错误', 
    5: isEnglish ? 'Filled' : '已成交',
    6: isEnglish ? 'PartialCancel' : '部分撤单', 
    7: isEnglish ? 'Trading' : '正在交易', // ing
    8: isEnglish ? 'Lost' : '丢失',
}

export const aliveOrderStatusList = [1, 2, 7]

export const SourceTypeConfig: SourceType = {
    'Stock': {
        name: isEnglish ? 'stock' : '股票',
        kfId: 1,
        color: ''
    },

    'Future': {
        name: isEnglish ? 'future': '期货',
        kfId: 2,
        color: 'danger'
    },

    'StockOption': {
        name: isEnglish ? 'option' : '期权',
        kfId: 3,
        color: 'info'
    },

    'Sim': {
        name: isEnglish ? 'simulation' : '模拟',
        kfId: 4,
        color: 'success'
    },
}

export const TaskTypeConfig: any = {
    'mFilter': {
        name: isEnglish ? 'market' : '行情',
        color: ''
    },

    'trade': {
        name: isEnglish ? 'trade' : '交易',
        color: 'info'
    }
}

export const HedgeFlag: NumberToStringObject = {
    0: isEnglish ? 'Speculation': '投机',
    1: isEnglish ? 'Arbitrage': '套利',
    2: isEnglish ? 'Hedge': '套保',
    // 3: isEnglish ? 'Covered': '备兑'
}

export const ExchangeIds: StringToStringObject = {
    "SSE": "上交所",
    "SZE": "深交所",
    "SHFE": "上期所",
    "DCE": "大商所",
    "CZCE": "郑商所",
    "CFFEX": "中金所",
    "INE": "能源中心"
}

export const InstrumentTypes: StringToNumberObject = {
    Unknown         :0,
    Stock           :1,
    Future          :2,
    Bond            :3,
    StockOption     :4,
    Fund            :5,
    TechStock       :6,
    Index           :7,
    Repo            :8
}


export const InstrumentType: NumberToStringObject = {
    0: isEnglish ? 'Unknown'     : '未知',
    1: isEnglish ? 'Stock'       : '普通股票',
    2: isEnglish ? 'Future'      : '期货',
    3: isEnglish ? 'Bond'        : '债券',
    4: isEnglish ? 'StockOption' : '股票期权',
    5: isEnglish ? 'Fund'        : '基金',
    6: isEnglish ? 'TechStock'   : '科创板股票',
    7: isEnglish ? 'Index'       : '指数',
    8: isEnglish ? 'Repo'        : '回购'
}

　　
// 最小交易单位限制了交易数量的连续性。最小交易单位目前国内市场买入股票或基金，
// 申报数量应当为100股或其整数倍。债券以人民币1000元面额为一手。
// 证券回购以1000元标准券或综合券为1手。债券和债券回购以1手或其整数倍进行申报，
// 其中，上交所债券回购以100手或其整数倍进行申报。
export const InstrumentTypeWithDetail: NumberToAnyObject = {
    0: {
        name: isEnglish ? 'Unknown'     : '未知',
    },

    1: {
        name: isEnglish ? 'Stock'       : '普通股票',
        miniTradeUnit: 100,
        sellToday: false,
        short: false
    },

    2: {
        name: isEnglish ? 'Future'      : '期货',
        miniTradeUnit: 1,
        sellToday: true,
        short: true
    },

    3: {
        name: isEnglish ? 'Bond'        : '债券',
    },

    4: {
        name: isEnglish ? 'StockOption' : '股票期权',
    },

    5: {
        name: isEnglish ? 'Fund'        : '基金',
        miniTradeUnit: 100,
        sellToday: false,
        short: false
    },

    6: {
        name: isEnglish ? 'TechStock'   : '科创板股票',
        miniTradeUnit: 100,
        sellToday: false,
        short: false
    },

    7: {
        name: isEnglish ? 'Index'       : '指数',
    },

    8: {
       name: isEnglish ? 'Repo'        : '回购',
    }
}

export const allowShorted = (instrumentType: number) => {
    if ([2, 4].includes(instrumentType)) return true;
    return false
}

export const PriceType: NumberToStringObject = {
    0: isEnglish ? 'Limit'       : '限价',
    1: isEnglish ? 'Market'      : '市价',
    2: isEnglish ? 'FakBest5'    : '上海深圳最优五档即时成交剩余撤销，不需要报价',
    3: isEnglish ? 'ForwardBest' : '深圳本方方最优价格申报', 
    4: isEnglish ? 'ReverseBest' : '上海最优五档即时成交剩余转限价',
    5: isEnglish ? 'Fak'         : '深圳即时成交剩余撤销',
    6: isEnglish ? 'Fok'         : '深圳市价全额成交或者撤销',
    7: isEnglish ? 'UnKnown'     : '未知'
}

export const VolumeCondition: NumberToStringObject = {
    0: isEnglish ? 'Any' : '任意', 
    1: isEnglish ? 'Min' : '最小', 
    2: isEnglish ? 'All' : '全部' 
};

export const TimeCondition: NumberToStringObject = { 
    0: isEnglish ? 'IOC' : 'IOC', 
    1: isEnglish ? 'GFD' : 'GFD', 
    2: isEnglish ? 'GTC' : 'GTC' 
};
