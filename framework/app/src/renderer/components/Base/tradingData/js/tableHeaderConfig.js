export const ordersHeader = (dateForHistory, moduleType) => {
    return  [
        {
            type: "text",
            label: "委托更新时间",
            prop: dateForHistory ? "updateTimeMMDD" : "updateTime",
            width: dateForHistory ? '180px' : '110px'
        },{
            type: "text",
            label: "代码",
            prop: "instrumentId",
            width: '60px'
        },{
            type: "text",
            label: "",
            prop: "side",
            width: '40px',
        },{
            type: "text",
            label: "",
            prop: "offset",
            width: '40px'
        },{
            type: "number",
            label: "委托价",
            prop: "limitPrice",
            width: '80px'
        },{
            type: "text",
            align: "center",
            label: "已成交/全部",
            prop: "volumeTraded",
            width: '80px'
        },{
            type: "text",
            label: "订单状态",
            prop: "statusName",
            width: '120px'
        },{
            type: 'number',
            label: "系统延迟(μs)",
            prop: "latencySystem", 
            width: '90px'
        },{
            type: 'number',
            label: "网络延迟(μs)",
            prop: "latencyNetwork", 
            width: '90px'
        },{
            type: "account-strategy",
            label: moduleType == 'account' ? '策略' : '账户',
            prop: moduleType == 'account' ? 'clientId' : 'accountId',
        },{
            type: 'operation',
            label: '',
            prop: 'oper',
            width: '40px'
        }
    ]
}


export const posHeader = (moduleType) => {
    
    if (moduleType === 'ticker') {
        return [
            {
                type: 'text',
                label: '账户',
                prop: 'accountIdResolved',
                width: '100px'
            },{
                type: 'number',
                label: '昨',
                prop: 'yesterdayVolume',
                flex: 1
            },{
                type: 'number',
                label: '今',
                prop: 'todayVolume',
                flex: 1
            },{
                type: 'number',
                label: '总',
                prop: 'totalVolume',
                flex: 1
            },{
                type: 'number',
                label: '开(持)仓均价',
                prop: 'avgPrice',
                flex: 1.2
            },{
                type: 'number',
                label: '最新价',
                prop: 'lastPrice',
                flex: 1.2
            },{
                type: 'number',
                label: '总成本',
                prop: 'totalPrice',
                flex: 1.5,
            },{
                type: 'number',
                label: '总市值',
                prop: 'totalMarketPrice',
                flex: 1.5,
            },{
                type: 'number',
                label: '浮动盈亏',
                prop: 'unRealizedPnl',
                flex: 1.5
            }
        ]
    }

    return [
        {
            type: 'text',
            label: '代码',
            prop: 'instrumentId',
            width: '60px'
        },{
            type: 'text',
            label: '',
            prop: 'direction',
            width: '35px'
        },{
            type: 'number',
            label: '昨',
            prop: 'yesterdayVolume',
            flex: 1
        },{
            type: 'number',
            label: '今',
            prop: 'todayVolume',
            flex: 1
        },{
            type: 'number',
            label: '总',
            prop: 'totalVolume',
            flex: 1
        },{
            type: 'number',
            label: '开(持)仓均价',
            prop: 'avgPrice',
            flex: 1.2
        },{
            type: 'number',
            label: '最新价',
            prop: 'lastPrice',
            flex: 1.2
        },{
            type: 'number',
            label: '总成本',
            prop: 'totalPrice',
            flex: 1.5,
        },{
            type: 'number',
            label: '总市值',
            prop: 'totalMarketPrice',
            flex: 1.5,
        },{
            type: 'number',
            label: '浮动盈亏',
            prop: 'unRealizedPnl',
            flex: 1.5
        }
    ]
}

export const tradesHeader = (dateForHistory, moduleType) => {
    return [{
        type: 'text',
        label: '柜台成交时间',
        prop: dateForHistory ? "updateTimeMMDD" : "updateTime",
        width: dateForHistory ? "180px" : '110px'
    },{
        type: 'text',
        label: '功夫系统时间',
        prop: dateForHistory ? "localUpdateTimeMMDD" : "localUpdateTime",
        width: dateForHistory ? "180px" : '110px'
    },{
        type: 'text',
        label: '代码',
        prop: 'instrumentId',
        width: '60px'
    },{
        type: 'text',
        label: '',
        prop: 'side',
        width: '60px'
    },{
        type: 'text',
        label: '',
        prop: 'offset',
        width: '40px'
    },{
        type: 'number',
        label: '成交价',
        prop: 'price',
        width: '80px'
    },{
        type: 'number',
        label: '成交量',
        prop: 'volume',
        width: '60px'
    },{
        type: 'number',
        label: "成交延迟(μs)",
        prop: "latencyTrade", 
        width: '90px'
    },{
        type: 'account-strategy',
        label: moduleType == 'account' ? '策略': '账户',
        prop: moduleType == 'account' ? 'clientId': 'accountId',
    }]
}