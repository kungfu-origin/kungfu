const blessed = require('blessed');
const contrib = require('blessed-contrib');
const screen = blessed.screen()
const utils = require('../public/utils');
const { getAccountList, getAccountOrder, getAccountTrade } = require('../../renderer/io/account');

var globalData = {
    account: {
        currentAccount: {}
    }
}


//create layout and widgets
const grid = new contrib.grid({rows: 12, cols: 12, screen: screen})

const accountTable = grid.set(0, 0, 4, 6, contrib.table,      {
        keys: true
        , fg: 'white'
        , selectedFg: 'white'
        , selectedBg: 'blue'
        , interactive: true
        , label: 'Active Processes'
        , width: '30%'
        , height: '30%'
        , border: {type: "line", fg: "cyan"}
        , columnSpacing: 10 //in chars
        , columnWidth: [16, 12, 12] /*in chars*/ })

const buildTdAccountList = (table) => {
    getAccountList().then(list => {
        const tableData = list.map(l => [l.account_id.toAccountId(), l.source_name, !!l.receive_md ? '✔' : ''])
        table.setData({headers: ['Account', 'Source', 'Md'], data: tableData})
        table.focus()
        screen.render()
    })
}

buildTdAccountList(accountTable)


const mdTable = grid.set(4, 0, 4, 3, contrib.table, {
    keys: true
    , fg: 'white'
    , selectedFg: 'white'
    , selectedBg: 'blue'
    , label: 'Active Processes'
    , width: '30%'
    , height: '30%'
    , border: {type: "line", fg: "cyan"}
    , columnSpacing: 10 //in chars
    , columnWidth: [8, 20] /*in chars*/ })

const buildMdAccountList = (table) => {
    getAccountList().then(list => {
        const tableData = list.filter(l => !!l.receive_md).map(l => [l.source_name, l.account_id.toAccountId()])
        table.setData({headers: ['Source', 'Account'], data: tableData})
        screen.render()
    })
}

buildMdAccountList(mdTable)




const pnl = grid.set(4, 3, 4, 3, contrib.sparkline, {
    label: 'pnl (daily)', 
    tags: true, 
    style: { fg: 'blue', titleFg: 'white' }
})

const orderTable = grid.set(8, 0, 4, 6, contrib.table, {
    key: true,
    fg: 'green',
    label: 'orders unfinished',
    columnSpacing: 1,
    columnWidth: [24, 10, 10]
})

// const dataOrder = (item) => {
//     return Object.freeze({
//         id: item.order_id.toString() + '_' + item.account_id.toString(),
//         insertTime: item.insert_time && moment(item.insert_time/1000000).format("YYYY-MM-DD HH:mm:ss"),
//         instrumentId: item.instrument_id,
//         side: sideName[item.side] ? sideName[item.side] : '--',
//         offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
//         limitPrice: item.limit_price,
//         volumeTraded: item.volume_traded + "/" + (item.volume),
//         statusName: orderStatus[item.status],
//         status: item.status,
//         clientId: item.client_id,
//         accountId: item.account_id,
//         orderId: item.order_id,
//         exchangeId: item.exchange_id
//     })
// };


const buildOrderTable = (table, accountId) => {
    getAccountOrder(accountId, {}, '20190527').then(list => {
        console.log(list)
        if(!list.length) return;
        const tableData = list.slice(0, 10).map(l => Object.values(dataOrder(l)))
        table.setData({headers: ['id', 'insertTime', 'instrumentId', 'side', 'offset', 'limitPrice', 'volumeTraded', 'statusName', 'clientId', 'orderId', 'exchangeId'], data: tableData})
        screen.render()
    })
}

buildOrderTable(orderTable, '15040900')




const posList = grid.set(0, 6, 8, 6, contrib.table, {
    key: true,
    fg: 'green',
    label: 'position',
    columnSpacing: 1,
    columnWidth: [24, 10, 10]
})



const tradeList = grid.set(8, 6, 4, 6, contrib.table, {
    key: true,
    fg: 'green',
    label: 'trades today',
    columnSpacing: 1,
    columnWidth: [24, 10, 10]
})

screen.render()


screen.key(['escape', 'q', 'C-c'], function(ch, key) {
    return process.exit(0);
  });
  