import blessed  from 'blessed';
import { line } from 'blessed-contrib'
import accountTable from '../public/AccountTable';
import posTable from '../public/PosTable';
import mdTable from '../public/MdTable';
import orderTable from '../public/OrderTable';
import tradeTable from '../public/TradeTable';
import Dashboard from '../public/Dashboard';

import { getAccountList, getAccountPos, getAccountOrder, getAccountTrade, getAccountAsset, getAccountPnlDay } from '@/io/account.js';
import { DEFAULT_PADDING, switchMd, switchTd, dealPnlData } from '../public/utils';
import { listProcessStatus } from '__gUtils/processUtils';

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;

class AccountDashboard extends Dashboard {
	constructor(){
		super()
		this.screen.title = 'Account Dashboard';
		this.globalData = {
			accountData: {},
			posData: {},
			mdData: {},
			pnlData: [],
			tradeData: [],
			orderData: [],
			processStatus: {},
			cashData: {},
			pnl: {
				x: [],
				y: [],
			}
		};
	}

	init(){
		const t = this;
		t.initAccountTable();
		t.initMdTable();
		t.initPosTable();
		t.initPnl();
		t.initOrderList();
		t.initTradeList();
		t.initBoxInfo();
		t.initMessage();
		t.screen.render();
		t.bindEvent();
	}
	
	initAccountTable(){
		const t = this;
		t.accountTable = accountTable.build({
			label: ' Trader Engines ',
			top: '0',
			parent: t.screen,
			left: '0%',
			width: WIDTH_LEFT_PANEL + '%',
			height: '33.33%',
			getDataMethod: getAccountList,
			afterSelectMethod: t._afterSelected.bind(t),
			afterSwitchMethod: t._afterSwitchAccountProcess.bind(t),
			style: {
				cell: {
					selected: {
						bold: true,
						bg: 'blue',
					},
				},
			}
	
		})
		t.accountTable.focus();
	}
	
	initMdTable(){
		const t = this;
		t.mdTable = mdTable.build({
			label: ' Market Engines ',
			parent: t.screen,
			top: '33.33%',
			left: '0',
			width: WIDTH_LEFT_PANEL / 2 + '%',
			height: '23.66%',
			getDataMethod: getAccountList,
			afterSwitchMethod: t._afterSwitchMdProcess.bind(t),
			style: {
				cell: {
					selected: {
						bold: true,
						bg: 'grey'
					},
				},
			}
		});
	}
	
	initPosTable(){
		const t = this;
		t.posTable = posTable.build({
			label: ' Positions ',
			parent: t.screen,
			top: '0',
			left: WIDTH_LEFT_PANEL + '%',
			width: 100 - WIDTH_LEFT_PANEL + '%',
			height: '55%',
			getDataMethod: getAccountPos
		})
	}
	
	initPnl(){
		const t = this;
		this.pnl = line({ 
			style:{ 
				line: "yellow", 
				text: "white", 
				baseline: "white",
			},
			xPadding: 5,
			border: {
				type: 'line',
			},
			padding: DEFAULT_PADDING,
			showLegend: false,
			wholeNumbersOnly: false, //true=do not show fraction in y axis
			label: 'Pnl',
			top: '33.33%',
			left: WIDTH_LEFT_PANEL / 2 + '%',
			width: WIDTH_LEFT_PANEL / 2 + '%',
			height: '23.66%',
			align: 'center'
		})
	  	this.screen.append(this.pnl) //must append before setting data		
	}
	
	initOrderList(){
		const t = this;
		t.orderTable = orderTable.build({
			label: ' Today Orders ',
			parent: t.screen,
			top: '56%',
			width: WIDTH_LEFT_PANEL - 5 + '%',
			height: '39%',
			getDataMethod: getAccountOrder
		});
	}
	
	initTradeList(){
		const t = this;
		t.tradeTable = tradeTable.build({
			label: ' Today Trades ',
			parent: t.screen,
			top: '56%',
			left: WIDTH_LEFT_PANEL - 5 + '%',
			width: 100 - WIDTH_LEFT_PANEL + 5.5 + '%',
			height: '39%',
			getDataMethod: getAccountTrade
		});
	}
	
	initBoxInfo() {
		const t = this;
		t.boxInfo = blessed.text({
			content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | Enter: process-switch | Double-Click/Enter: select ',
			parent: t.screen,		
			left: '0%',
			top: '95%',
			width: '100%',
			height: '6%',
			valign: 'middle',
			tags: true
		});	
	}
	
	refresh(){
		const t = this;
		const { processStatus, accountData, mdData, posData, orderData, tradeData, cashData } = t.globalData;
		t.accountTable.refresh(accountData, processStatus, cashData)
		t.mdTable.refresh(mdData, processStatus)
		t.posTable.refresh(posData)
		t.orderTable.refresh(orderData)
		t.tradeTable.refresh(tradeData);
		t.pnl.setData({
			title: '',
			x: t.globalData.pnl.x || [],
			y: t.globalData.pnl.y || [],
		})
	}
	
	getData(){
		const  t = this;
		let timer = null;
		const runPromises = () => {
			clearTimeout(timer)
			const currentId = Object.keys(t.globalData.accountData)[t.accountTable.selectedIndex || 0];
			//md + td
			const mdTdPromise = t.accountTable.getData(t.globalData)
			.then(({accountData, mdData}) => {
				t.globalData[mdData] = mdData;
				t.globalData[accountData] = accountData;
			})
			.then(() => t.refresh())
			//account assets
			const accountAssetsPromises = Object.keys(t.globalData.accountData || {})
			.map(accountId => getAccountAsset(accountId)
			.then(cash => t.globalData.cashData[accountId] = ((cash || [])[0] || {}))
			.then(() => t.refresh()))
			//pos
			const posDataPromise = t.posTable.getData(currentId)
			.then(pos => t.globalData.posData = pos || {})
			.then(() => t.refresh())
			//order
			const orderDataPromise = t.orderTable.getData(currentId)
			.then(orders => t.globalData.orderData = orders || [])
			.then(() => t.refresh())
			//trades
			const tradeDataPromise = t.tradeTable.getData(currentId)
			.then(trades => t.globalData.tradeData = trades || [])
			.then(() => t.refresh())
			//pnl
			const pnlPromise = getAccountPnlDay(currentId)
			.then(data => {
				t.globalData.pnl = dealPnlData(data)
				t.refresh()
			})
			.catch(err => {})

			Promise.all([
				mdTdPromise, 
				...accountAssetsPromises, 
				posDataPromise,
				orderDataPromise,
				tradeDataPromise,
				pnlPromise
			]).finally(() => {
				timer = setTimeout(() => runPromises(), 3000)
			})
		}
		runPromises()
	}
	
	render(){
		const t = this;
		t.screen.render();
		// async refresh of the ui
		setInterval(function () {
			t.screen.render();
		}, 300);
	}
	
	bindEvent(){
		const t = this;
		let i = 0;
		let boards = ['accountTable', 'mdTable', 'posTable', 'orderTable', 'tradeTable'];
		t.screen.key(['left', 'right'], (ch, key) => {
			(key.name === 'left') ? i-- : i++;
			if (i === 5) i = 0;
			if (i === -1) i = 4;
			t[boards[i]].focus();
		});
	
		t.screen.key(['escape', 'q', 'C-c'], function(ch, key) {
			t.screen.destroy();
			process.exit(0);
		});	
	}

	_afterSwitchAccountProcess(index){
		const t = this;
		const tdProcess = Object.values(t.globalData.accountData || {})[index];
		switchTd(tdProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
	}

	_afterSwitchMdProcess(index){
		const t = this;
		const mdProcess = Object.values(t.globalData.mdData || {})[index];
		switchMd(mdProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
	}

	_afterSelected(){
		this.globalData = {
			...this.globalData,
			posData: {},
			pnlData: [],
			tradeData: [],
			orderData: []
		};
		this.refresh();
	}
	
	
	getProcessStatus(){
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
}
	


export default () => {
	const accountDashboard = new AccountDashboard();
	accountDashboard.init();
	accountDashboard.render();
	accountDashboard.getData()
	accountDashboard.refresh();
	accountDashboard.getProcessStatus();
	setInterval(() => {
		accountDashboard.refresh();
	}, 1000)	
}




