import blessed  from 'blessed';
import accountTable from '../public/AccountTable';
import posTable from '../public/PosTable';
import mdTable from '../public/MdTable';
import orderTable from '../public/OrderTable';
import tradeTable from '../public/TradeTable';

import { getAccountList, getAccountPos, getAccountOrder, getAccountAsset } from '@/io/account.js';
import { listProcessStatus } from '__gUtils/processUtils';

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;

function AccountDashboard(){
	this.screen = blessed.screen({
		smartCSR: true,
		fullUnicode: true,
		autoPadding: true,
	});
	this.screen.title = 'Account Dashboard';
	this.globalData = {
		accountData: {},
		posData: {},
		mdData: {},
		pnlData: [],
		tradeData: [],
		orderData: [],
		processStatus: {},
		cashData: {}
	};
}

AccountDashboard.prototype.init = function(){
	const t = this;
	t.initAccountTable();
	t.initMdTable();
	t.initPosTable();
	t.initLogo();
	t.initOrderList();
	t.initTradeList();
	t.initBoxInfo();
	t.screen.render();
	t.bindEvent();
}

AccountDashboard.prototype.initAccountTable = function(){
	const t = this;
	t.accountTable = accountTable.build({
		label: ' Trader Engines ',
		top: '0',
		parent: t.screen,
		left: '0%',
		width: WIDTH_LEFT_PANEL + '%',
		height: '33.33%',
		getDataMethod: getAccountList,
		afterSelectMethod: t.clearTraderData.bind(t),
		style: {
			cell: {
				selected: {
					bold: true,
					bg: 'blue',
					fg: 'white'
				},
			},
		}

	})
	t.accountTable.focus();
}

AccountDashboard.prototype.initMdTable = function(){
	const t = this;
	t.mdTable = mdTable.build({
		label: ' Market Engines ',
		parent: t.screen,
		top: '33.33%',
		left: '0',
		width: WIDTH_LEFT_PANEL / 2 + '%',
		height: '23.66%',
		getDataMethod: getAccountList,
		style: {
			cell: {
				selected: {
					bold: true,
					bg: 'blue',
					fg: 'white'
				},
			},
		}
	});
}

AccountDashboard.prototype.initPosTable = function(){
	const t = this;
	t.posTable = posTable.build({
		label: ' Positions ',
		parent: t.screen,
		top: '0',
		left: WIDTH_LEFT_PANEL + '%',
		width: 100 - WIDTH_LEFT_PANEL + '%',
		height: '56%',
		getDataMethod: getAccountPos
	})
}

AccountDashboard.prototype.initLogo = function(){
	const t = this;
	t.logo = blessed.text({
		parent: t.screen,
		content: ' \n kungfu trader \n version: 2.0.0',
		top: '34.33%',
		left: WIDTH_LEFT_PANEL / 2 + '%',
		width: WIDTH_LEFT_PANEL / 2 + '%',
		height: '18.66%',
		align: 'center'

	})
}

AccountDashboard.prototype.initOrderList = function(){
	const t = this;
	t.orderTable = orderTable.build({
		label: ' Current Orders ',
		parent: t.screen,
		top: '56%',
		width: WIDTH_LEFT_PANEL - 5 + '%',
		height: '39%',
		getDataMethod: getAccountOrder
	});
}

AccountDashboard.prototype.initTradeList = function(){
	const t = this;
	t.tradeTable = tradeTable.build({
		label: ' Today Trades ',
		parent: t.screen,
		top: '56%',
		left: WIDTH_LEFT_PANEL - 5 + '%',
		width: 100 - WIDTH_LEFT_PANEL + 5 + '%',
		height: '39%',
		getDataMethod: getAccountOrder
	});
}

AccountDashboard.prototype.initBoxInfo = function() {
	const t = this;
	t.boxInfo = blessed.text({
		content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | s: process-start | x: process-stop ',
		parent: t.screen,		
		left: '0%',
		top: '95%',
		width: '100%',
		height: '6%',
		valign: 'middle',
		tags: true
	});	
}

AccountDashboard.prototype.refresh = function(){
	const t = this;
	const { processStatus, accountData, mdData, posData, orderData, tradeData, cashData } = t.globalData;
	t.accountTable.refresh(accountData, processStatus, cashData)
	t.mdTable.refresh(mdData)
	t.posTable.refresh(posData)
	t.orderTable.refresh(orderData)
	t.tradeTable.refresh(tradeData);
}

AccountDashboard.prototype.getData = function(){
	const  t = this;
	const currentId = Object.keys(t.globalData.accountData)[t.accountTable.selectedIndex || 0];
	
	//md + td
	const getAccountListPromise = t.accountTable.getData(t.globalData).then(({accountData, mdData}) => {
		t.globalData[mdData] = mdData;
		t.globalData[accountData] = accountData;
	})

	//account assets
	const getAccountAssetPromise = Object.keys(t.globalData.accountData || {}).map(accountId => getAccountAsset(accountId).then(cash => t.globalData.cashData[accountId] = ((cash || [])[0] || {})))
	
	//pos
	const getPosPromise = t.posTable.getData(currentId).then(pos => t.globalData.posData = pos || {})
	//order
	const getOrderPromise = t.orderTable.getData(currentId).then(orders => t.globalData.orderData = orders || [])
	//all
	Promise.all([getAccountListPromise, getPosPromise, getOrderPromise, getAccountAssetPromise]).then(() => {
		t.refresh()
	})
}

AccountDashboard.prototype.clearTraderData = function(){
	this.globalData = {
		...this.globalData,
		posData: {},
		pnlData: [],
		tradeData: [],
		orderData: []
	};
	this.refresh();
}

AccountDashboard.prototype.render = function(){
	const t = this;
	t.screen.render();
	// async refresh of the ui
	setInterval(function () {
		t.screen.render();
	}, 300);
}

AccountDashboard.prototype.bindEvent = function(){
	const t = this;
	let i = 0;
	let boards = ['accountTable', 'mdTable', 'logo','posTable', 'orderTable', 'tradeTable'];
	t.screen.key(['left', 'right'], (ch, key) => {
		(key.name === 'left') ? i-- : i++;
		if (i === 6) i = 0;
		if (i === -1) i = 5;
		t[boards[i]].focus();
	});

	t.screen.key(['escape', 'q', 'C-c'], function(ch, key) {
		t.screen.destroy();
		process.exit(0);
	});	
}

AccountDashboard.prototype.getProcessStatus = function(){
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
	startGetProcessStatus()
}

const accountDashboard = new AccountDashboard();
accountDashboard.init();
accountDashboard.render();
accountDashboard.getData();
accountDashboard.getProcessStatus();
setInterval(() => {
	accountDashboard.getData();
}, 3000)
setInterval(() => {
	accountDashboard.refresh();
}, 1000)




