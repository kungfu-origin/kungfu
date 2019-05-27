const blessed = require('blessed');
const contrib = require('blessed-contrib');
const screen = blessed.screen()
const utils = require('../public/utils');

var globalData = {
    account: {
        currentAccount: {}
    }
}


//create layout and widgets
const grid = new contrib.grid({rows: 12, cols: 12, screen: screen})

const accountTable = grid.set(0, 0, 4, 6, contrib.table,      {
    keys: true,
    fg: 'white',
    selectedFg: 'white',
    selectedBg: 'blue',
    interactive: true,
    label: 'Trading Account Processes',
    width: '30%',
    height: '30%',
    border: {type: "line", fg: "cyan"},
    columnSpacing: 10, //in chars
    columnWidth: [16, 12, 12] /*in chars*/ })

const buildAccountList = (table) => {
    const { getAccountList } = require('../../renderer/io/account');
    getAccountList().then(list => {
        const tableData = list.map(l => [l.account_id.toAccountId(), l.source_name, !!l.receive_md ? '✔' : ''])
        table.setData({headers: ['Account', 'Source', 'Md',], data: tableData})
        screen.render()
        table.focus()
    })
}

buildAccountList(accountTable)

const mdList = grid.set(4, 0, 4, 3, contrib.table, {
    key: true,
    fg: 'green',
    label: 'Marketing Processes',
    columnSpacing: 1,
    columnWidth: [24, 10, 10] 
})

const pnl = grid.set(4, 3, 4, 3, contrib.sparkline, {
    label: 'pnl (daily)', 
    tags: true, 
    style: { fg: 'blue', titleFg: 'white' }
})

const orderList = grid.set(8, 0, 4, 6, contrib.table, {
    key: true,
    fg: 'green',
    label: 'orders unfinished',
    columnSpacing: 1,
    columnWidth: [24, 10, 10]
})

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
  