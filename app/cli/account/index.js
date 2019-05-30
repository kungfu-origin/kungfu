import blessed  from 'blessed';
import accountTable from '../public/accountTable';
import posTable from '../public/posTable';
import mdTable from '../public/mdTable';
import orderTable from '../public/orderTable';

import { offsetName, orderStatus, sideName, posDirection, parseToString, TABLE_BASE_OPTIONS } from "../public/utils";
import { getAccountList, getAccountPos, getAccountOrder } from '@/io/account.js';
import { listProcessStatus, startMaster } from '__gUtils/processUtils';


// 定义全局变量
const WIDTH_LEFT_PANEL = 60;



//定义Dashboard
function Dashboard(){
	const t = this;
	t.screen = blessed.screen({
		smartCSR: true,
		fullUnicode: true,
		autoPadding: true,
	});

	t.screen.title = 'Account Dashboard';

	t.orderTableHeaders = ['OrderTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 'Strat']
	t.tradeTableHeaders = ['TradeTime', 'Ticker', 'Side', 'Offset', 'Price', 'Vol', 'Strat']

	t.globalData = {
		accountData: {},
		posData: {},
		mdData: {},
		pnlData: [],
		tradeData: [],
		orderData: [],
		processStatus: {},
	};

	t.currentAccount = '15040900'

}

Dashboard.prototype.init = function(){
	const t = this;
	t.initAccountTable();
	t.initMdTable();
	t.initPosTable();
	t.initPnlList();
	t.initOrderList();
	t.initTradeList();
	t.initBoxInfo();
	t.screen.render()
}

Dashboard.prototype.initAccountTable = function(){
	const t = this;
	t.accountTable = accountTable.build({
		label: ' Trader Engines ',
		top: '0',
		parent: t.screen,
		left: '0%',
		width: WIDTH_LEFT_PANEL + '%',
		height: '33.33%',
		getDataMethod: getAccountList

	})
	t.accountTable.focus();
}

Dashboard.prototype.initMdTable = function(){
	const t = this;
	t.mdTable = mdTable.build({
		label: ' Market Engines ',
		parent: t.screen,
		top: '33.33%',
		left: '0',
		width: WIDTH_LEFT_PANEL / 2 + '%',
		height: '23.66%',
		getDataMethod: getAccountList
	});
}

Dashboard.prototype.initPosTable = function(){
	const t = this;
	t.posTable = posTable.build({
		parent: t.screen,
		top: '0',
		left: WIDTH_LEFT_PANEL + '%',
		width: 100 - WIDTH_LEFT_PANEL + '%',
		height: '56%',
		getDataMethod: getAccountPos
	})
}

Dashboard.prototype.initPnlList = function(){
	const t = this;
	t.pnl = blessed.list({
		label: ' Pnl ',
		parent: t.screen,
		content: '',
		top: '33.33%',
		left: WIDTH_LEFT_PANEL / 2 + '%',
		width: WIDTH_LEFT_PANEL / 2 + '%',
		height: '23.66%',
		padding: 0,
		scrollbar: {
			ch: ' ',
			inverse: false
		},
		border: {
			type: 'line'
		},
		keys: true,
		autoCommandKeys: true,
		tags: true,
		style: {
			...TABLE_BASE_OPTIONS['style'],			
		}
	});
}

Dashboard.prototype.initOrderList = function(){
	const t = this;
	t.orderTable = orderTable.build({
		label: ' Current Orders ',
		parent: t.screen,
		top: '56%',
		width: WIDTH_LEFT_PANEL - 5 + '%',
		height: '39%',
		style: {
			...TABLE_BASE_OPTIONS['style'],
		},
		headers: [...t.orderTableHeaders]
	});
}

Dashboard.prototype.initTradeList = function(){
	const t = this;
	t.tradeTable = table({
		label: ' Today Trades ',
		parent: t.screen,
		top: '56%',
		left: WIDTH_LEFT_PANEL - 5 + '%',
		width: 100 - WIDTH_LEFT_PANEL + 5 + '%',
		height: '39%',
		style: {
			...TABLE_BASE_OPTIONS['style'],
		},
		headers: [...t.tradeTableHeaders]
	});
}

Dashboard.prototype.initBoxInfo = function() {
	const t = this;
	t.boxInfo = blessed.text({
		content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit{|} {cyan-fg}{bold}',
		parent: t.screen,		
		left: '0%',
		top: '95%',
		width: '100%',
		height: '6%',
		valign: 'middle',
		tags: true,
		style: {
			...TABLE_BASE_OPTIONS['style'],	

		}
	});	
}

Dashboard.prototype.bindEvent = function(){
	const t = this;
	let i = 0;
	let boards = ['accountTable', 'mdTable', 'pnl','posTable', 'orderTable', 'tradeTable'];
	t.screen.key(['left', 'right'], (ch, key) => {
		(key.name === 'left') ? i-- : i++;
		if (i === 6) i = 0;
		if (i === -1) i = 5;
		t[boards[i]].focus();
		t[boards[i]].style.border.fg = 'blue';
		if (key.name === 'left') {
		  if (i == 3)
			t[boards[0]].style.border.fg = 'white';
		  else
			t[boards[i + 1]].style.border.fg = 'white';
		}
		else {
		   if (i == 0)
			t[boards[3]].style.border.fg = 'white';
		  else
			t[boards[i - 1]].style.border.fg = 'white';
		}
	});

	t.screen.key(['escape', 'q', 'C-c'], function(ch, key) {
		t.screen.destroy();
		process.exit(0);
	});	
}

Dashboard.prototype.render = function(){
	const t = this;
	t.screen.render();
	// async refresh of the ui
	setInterval(function () {
		t.screen.render();
	}, 300);
}

Dashboard.prototype.refresh = function(){
	const t = this;
	const { processStatus, accountData, mdData, posData, orderData } = t.globalData;
	t.accountTable.refresh(accountData, processStatus)
	t.mdTable.refresh(mdData)
	t.posTable.refresh(posData)
	t.posTable.refresh(orderData)
	// const tradeListData = []
	// tradeListData.unshift(t.tradeTableHeaders)
	// t.tradeList.setData(tradeListData);
}

Dashboard.prototype.getData = function(){
	const  t = this;

	const getAccountListPromise = t.accountTable.getData(t.globalData).then(({accountData, mdData}) => {
		t.globalData[mdData] = mdData;
		t.globalData[accountData] = accountData;
	})

	//pos
	const getPosPromise = t.posTable.getData(t.currentAccount).then(pos => {
		t.globalData.posData = pos || {}
	})

	//order
	// const getOrderPromise = getAccountOrder(t.currentAccount, {}).then(orders => {
	// 	t.globalData.orderData = orders || []

	// })

	Promise.all([getAccountListPromise, getPosPromise]).then(() => {
		t.refresh()
	})
}

Dashboard.prototype.startMaster = function(){
	const t = this;
	//循环获取processStatus
	var listProcessTimer;
	const startGetProcessStatus = () => {
		clearTimeout(listProcessTimer)
		listProcessStatus()
		.then(res => {
			t.globalData.processStatus = Object.freeze(res);
		})
		.catch(err => console.error(err))
		.finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
	}

	//start pm2 kungfu master
	startMaster(false)
	.catch(() => {})
	.finally(() => {
		startGetProcessStatus()
	})
}





const accountDashboard = new Dashboard();
accountDashboard.init();
accountDashboard.render();
accountDashboard.bindEvent();
accountDashboard.getData();

setInterval(() => {
	// accountDashboard.getData();
}, 3000)
setInterval(() => {
	// accountDashboard.fresh();
}, 1000)
// accountDashboard.startMaster();



