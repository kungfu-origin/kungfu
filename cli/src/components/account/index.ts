// import accountTable from '@/assets/components/AccountTable';
import posTable from '@/assets/components/PosTable';
// import mdTable from '@/assets/components/MdTable';
import orderTable from '@/assets/components/OrderTable';
import tradeTable from '@/assets/components/TradeTable';
import Dashboard from '@/assets/components/Dashboard';

import { DEFAULT_PADDING, getStatus } from '@/assets/scripts/utils';
import { switchTd, switchMd } from '__io/actions/account';

import { logger } from '__gUtils/logUtils';

const blessed = require('blessed');

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;

class AccountDashboard extends Dashboard {
	accountId: string
	screen: any;
	globalData: {
		assetData: any,
		orders: any,
		trades: any[],
		pos: any,

	};

	boards: any;

	constructor(accountId: string){
		super()
		this.screen.title = 'Account Dashboard';
		this.accountId = accountId;
		this.globalData = {
			assetData: {},
			orders: {},
			trades: [],
			pos: {},
		};

		this.boards = {};
		this.init()
	}

	init(){
		const t = this;
		// t.initAccountTable();
		// t.initMdTable();

		t.initPosTable();
		// t.initPnl();
		t.initOrderList();
		t.initTradeList();
		t.initBoxInfo();
		t.screen.render();
		t.bindEvent();
	}
	
	// initAccountTable(){
	// 	const t = this;
	// 	t.boards.accountTable = accountTable.build({
	// 		label: ' Trader Engines ',
	// 		top: '0',
	// 		parent: t.screen,
	// 		left: '0%',
	// 		width: WIDTH_LEFT_PANEL + '%',
	// 		height: '33.33%',
	// 		getDataMethod: getAccountList,
	// 		mouse: false,						
	// 		style: {
	// 			cell: {
	// 				selected: {
	// 					bold: true,
	// 					bg: 'blue',
	// 				},
	// 			},
	// 		}
	
	// 	})
	// 	t.boards.accountTable.focus();
	// }
	
	// initMdTable(){
	// 	const t = this;
	// 	t.mdTable = mdTable.build({
	// 		label: ' Market Engines ',
	// 		parent: t.screen,
	// 		top: '33.33%',
	// 		left: '0',
	// 		width: WIDTH_LEFT_PANEL / 2 + '%',
	// 		height: '23.66%',
	// 		getDataMethod: getAccountList,
	// 		afterSwitchMethod: t._afterSwitchMdProcess.bind(t),
	// 		style: {
	// 			cell: {
	// 				selected: {
	// 					bold: true,
	// 					bg: 'grey'
	// 				},
	// 			},
	// 		}
	// 	});
	// }
	
	initPosTable(){
		const t = this;
		t.boards.posTable = posTable().build({
			label: ' Positions ',
			parent: t.screen,
			top: '0',
			left: WIDTH_LEFT_PANEL - 5 + '%',
			width: 100 - WIDTH_LEFT_PANEL + 5.5 + '%',
			height: '45%-1'
		})
	}
	
	initOrderList(){
		const t = this;
		t.boards.orderTable = orderTable('account').build({
			label: ' Today Orders ',
			parent: t.screen,
			top: '45%',
			width: WIDTH_LEFT_PANEL - 5 + '%',
			height: '45%'
		});
	}
	
	initTradeList(){
		const t = this;
		t.boards.tradeTable = tradeTable('account').build({
			label: ' Today Trades ',
			parent: t.screen,
			top: '45%',
			left: WIDTH_LEFT_PANEL - 5 + '%',
			width: 100 - WIDTH_LEFT_PANEL + 5.5 + '%',
			height: '45%'
		});
	}
	
	initBoxInfo() {
		const t = this;
		t.boards.boxInfo = blessed.text({
			content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | Enter: process-switch',
			parent: t.screen,		
			left: '0%',
			top: '95%',
			width: '100%',
			height: '6%',
			valign: 'middle',
			tags: true
		});	
	}
	
	// refresh(){
	// 	const t = this;
	// 	const { processStatus, accountData, mdData, posData, orderData, tradeData, cashData } = t.globalData;
	// 	t.accountTable.refresh(accountData, processStatus, cashData)
	// 	t.mdTable.refresh(mdData, processStatus)
	// 	t.posTable.refresh(posData)
	// 	t.orderTable.refresh(orderData)
	// 	t.tradeTable.refresh(tradeData);
	// 	t.pnl.setData({
	// 		title: '',
	// 		x: t.globalData.pnl.x || [],
	// 		y: t.globalData.pnl.y || [],
	// 	})
	// }
	
	// getData(){
	// 	const  t = this;
	// 	let timer = null;
	// 	const runPromises = () => {
	// 		clearTimeout(timer)
	// 		const currentId = Object.keys(t.globalData.accountData)[t.accountTable.selectedIndex || 0];
	// 		if(!currentId) timer = setTimeout(() => runPromises(), 3000)

	// 		//md + td
	// 		const mdTdPromise = t.accountTable.getData(t.globalData)
	// 		.then(({accountData, mdData}) => {
	// 			t.globalData[mdData] = mdData;
	// 			t.globalData[accountData] = accountData;
	// 		})
	// 		.then(() => t.refresh())
	// 		//account assets
	// 		const accountAssetsPromises = Object.keys(t.globalData.accountData || {})
	// 		.map(accountId => getAccountAsset(accountId)
	// 		.then(cash => t.globalData.cashData[accountId] = ((cash || [])[0] || {}))
	// 		.then(() => t.refresh()))
	// 		//pos
	// 		const posDataPromise = t.posTable.getData(currentId)
	// 		.then(pos => t.globalData.posData = pos || {})
	// 		.then(() => t.refresh())
	// 		//order
	// 		const orderDataPromise = t.orderTable.getData(currentId)
	// 		.then(orders => t.globalData.orderData = orders || [])
	// 		.then(() => t.refresh())
	// 		//trades
	// 		const tradeDataPromise = t.tradeTable.getData(currentId)
	// 		.then(trades => t.globalData.tradeData = trades || [])
	// 		.then(() => t.refresh())
	// 		//pnl
	// 		// const pnlPromise = getAccountPnlDay(currentId)
	// 		// .then(data => {
	// 			// t.globalData.pnl = dealPnlData(data)
	// 			// t.refresh()
	// 		// })
	// 		// .catch(err => {})

	// 		Promise.all([
	// 			mdTdPromise, 
	// 			...accountAssetsPromises, 
	// 			posDataPromise,
	// 			orderDataPromise,
	// 			tradeDataPromise,
	// 			pnlPromise
	// 		]).finally(() => {
	// 			timer = setTimeout(() => runPromises(), 3000)
	// 		})
	// 	}
	// 	runPromises()
	// }
	
	// render(){
	// 	const t = this;
	// 	t.screen.render();
	// 	// async refresh of the ui
	// 	setInterval(function () {
	// 		t.screen.render();
	// 	}, 300);
	// }
	
	bindEvent(){
		const t = this;
		let i = 0;
		let boards = ['accountTable', 'mdTable', 'posTable', 'orderTable', 'tradeTable'];
		t.screen.key(['left', 'right'], (ch: any, key: any) => {
			(key.name === 'left') ? i-- : i++;
			if (i === 5) i = 0;
			if (i === -1) i = 4;
			t.boards[boards[i]].focus();
		});
	
		t.screen.key(['escape', 'q', 'C-c'], function() {
			t.screen.destroy();
			process.exit(0);
		});	
	}

	// _afterSwitchAccountProcess(index){
	// 	const t = this;
	// 	const pStatus = t.globalData.processStatus;
	// 	const tdProcess = Object.values(t.globalData.accountData || {})[index];
	// 	switchTd(tdProcess, getStatus(`td_${tdProcess.account_id}`, pStatus)).then(() => {t.message.log(' operation sucess!', 2)})
	// }

	// _afterSwitchMdProcess(index){
	// 	const t = this;
	// 	const pStatus = t.globalData.processStatus;
	// 	const mdProcess = Object.values(t.globalData.mdData || {})[index];
	// 	switchMd(mdProcess, getStatus(`md_${mdProcess.source_name}`, pStatus)).then(() => {t.message.log(' operation sucess!', 2)})
	// }

	// _afterSelected(){
	// 	this.globalData = {
	// 		...this.globalData,
	// 		posData: {},
	// 		pnlData: [],
	// 		tradeData: [],
	// 		orderData: []
	// 	};
	// 	this.refresh();
	// }
	
	
	// getProcessStatus(){
	// 	const t = this;
	// 	//循环获取processStatus
	// 	var listProcessTimer;
	// 	const startGetProcessStatus = () => {
	// 		clearTimeout(listProcessTimer)
	// 		listProcessStatus()
	// 		.then(res => {
	// 			t.globalData.processStatus = Object.freeze(res);
	// 		})
	// 		.catch(err => console.error(err))
	// 		.finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
	// 	}
	// 	startGetProcessStatus()
	// }
}
	


export default (accountId: string) => {
	const accountDashboard = new AccountDashboard(accountId);
	// accountDashboard.init();
	// accountDashboard.render();
	// accountDashboard.refresh();
	// accountDashboard.getProcessStatus();
	// accountDashboard.getData();
	// setInterval(() => {
	// 	accountDashboard.refresh();
	// }, 1000)	
}




