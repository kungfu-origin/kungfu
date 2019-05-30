import blessed  from 'blessed';
import accountTable from '../public/AccountTable';
import posTable from '../public/PosTable';
import mdTable from '../public/MdTable';
import orderTable from '../public/OrderTable';
import tradeTable from '../public/TradeTable';

import { offsetName, orderStatus, sideName, posDirection, parseToString, TABLE_BASE_OPTIONS } from "../public/utils";
import { getAccountList, getAccountPos, getAccountOrder, getMdTdState, getAccountAsset } from '@/io/account.js';
import { listProcessStatus } from '__gUtils/processUtils';


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
	
	t.globalData = {
		accountData: {},
		posData: {},
		mdData: {},
		pnlData: [],
		tradeData: [],
		orderData: [],
		processStatus: {},
		cashData: {}
	};

	t.currentId = null;

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

Dashboard.prototype.initMdTable = function(){
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

Dashboard.prototype.initPosTable = function(){
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
		mouse: true,
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
		getDataMethod: getAccountOrder
	});
}

Dashboard.prototype.initTradeList = function(){
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

Dashboard.prototype.initBoxInfo = function() {
	const t = this;
	t.boxInfo = blessed.text({
		content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | a: process-start | x: process-stop ',
		parent: t.screen,		
		left: '0%',
		top: '95%',
		width: '100%',
		height: '6%',
		valign: 'middle',
		tags: true
	});	
}

Dashboard.prototype.refresh = function(){
	const t = this;
	const { processStatus, accountData, mdData, posData, orderData, tradeData, cashData } = t.globalData;
	t.accountTable.refresh(accountData, processStatus, cashData)
	t.mdTable.refresh(mdData)
	t.posTable.refresh(posData)
	t.orderTable.refresh(orderData)
	t.tradeTable.refresh(tradeData);
}

Dashboard.prototype.getData = function(){
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

Dashboard.prototype.clearTraderData = function(){
	this.globalData = {
		...this.globalData,
		posData: {},
		pnlData: [],
		tradeData: [],
		orderData: []
	};
	this.refresh();
}

Dashboard.prototype.render = function(){
	const t = this;
	t.screen.render();
	// async refresh of the ui
	setInterval(function () {
		t.screen.render();
	}, 300);
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
	});

	t.screen.key(['escape', 'q', 'C-c'], function(ch, key) {
		t.screen.destroy();
		process.exit(0);
	});	
}

Dashboard.prototype.getProcessStatus = function(){
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


const accountDashboard = new Dashboard();
accountDashboard.init();
accountDashboard.render();
accountDashboard.bindEvent();
accountDashboard.getData();
accountDashboard.getProcessStatus();

setInterval(() => {
	accountDashboard.getData();
}, 3000)
setInterval(() => {
	accountDashboard.refresh();
}, 1000)




