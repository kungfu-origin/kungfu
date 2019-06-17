import blessed  from 'blessed';
import path from 'path';
import { Tail } from 'tail';

import strategyTable from '__@/assets/components/StrategyTable';
import posTable from '__@/assets/components/PosTable';
import orderTable from '__@/assets/components/OrderTable';
import tradeTable from '__@/assets/components/TradeTable';
import Dashboard from '__@/assets/components/Dashboard';

import { getStrategyList, getStrategyPos, getStrategyTrade, getStrategyOrder, getStrategysPnl } from '@/io/strategy.js';
import { TABLE_BASE_OPTIONS, DEFAULT_PADDING, switchStrategy, dealLog, buildTradingDay } from '__@/assets/utils';
import { dealLogMessage, getLog } from '@/assets/js/utils';
import { LOG_DIR } from '__gConfig/pathConfig';
import { listProcessStatus } from '__gUtils/processUtils';

// 定义全局变量
const WIDTH_LEFT_PANEL = 50;

class StrategyDashboard extends Dashboard {
	constructor(){
		super()
		this.screen.title = 'Account Dashboard';
		this.globalData = {
			strategyData: {},
			posData: {},
			pnlData: {},
			tradeData: [],
			orderData: [],
			processStatus: {},

		};
		this.logWatcher = null;
	}

	init(){
		const t = this;
		t.initStrategyTable();
        t.initLogTable();
        t.initPostList();
		t.initOrderList();
		t.initTradeList();
		t.initBoxInfo();
		t.initMessage();
		t.screen.render();
	}
	
	initStrategyTable(){
		const t = this;
		t.strategyTable = strategyTable.build({
			label: ' Strategies ',
			top: '0',
			parent: t.screen,
			left: '0%',
			width: WIDTH_LEFT_PANEL + '%',
			height: '33.33%',
			getDataMethod: getStrategyList,
			afterSelectMethod: t._afterSelected.bind(t),
			afterSwitchMethod: t._afterSwitchStrategyProcess.bind(t),
			style: {
				cell: {
					selected: {
						bold: true,
						bg: 'blue',
					},
				},
			}
	
		})
		t.strategyTable.focus();
	}

	initLogTable(){
		const t = this;
		t.logTable = blessed.log({
            ...TABLE_BASE_OPTIONS,
			label: ' Logs ',
			parent: t.screen,
			top: '33.33%',
            left: '0',
            padding: DEFAULT_PADDING,
			width: WIDTH_LEFT_PANEL + '%',
			height: '62.66%',
			style: {
				...TABLE_BASE_OPTIONS.style,
			}
        })
	}
    
    	
	initPostList(){
		const t = this;
		t.posTable = posTable.build({
			label: ' Positions ',
			parent: t.screen,
            top: '0%',
            left: WIDTH_LEFT_PANEL + '%',
			width: 100 - WIDTH_LEFT_PANEL + '%',
			height: '33.33%',
			getDataMethod: getStrategyPos
		});
	}
	
	
	initOrderList(){
		const t = this;
		t.orderTable = orderTable.build({
			label: ' Today Orders ',
			parent: t.screen,
			top: '33.33%',
            left: WIDTH_LEFT_PANEL + '%',
			width: 100 - WIDTH_LEFT_PANEL + '%',
			height: '33.33%',
            getDataMethod: getStrategyOrder,
            pad: 1,
            headers: ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 'AccountId']
		});
	}
	
	initTradeList(){
		const t = this;
		t.tradeTable = tradeTable.build({
			label: ' Today Trades ',
			parent: t.screen,
			top: '66.66%',
            left: WIDTH_LEFT_PANEL + '%',
			width: 100 - WIDTH_LEFT_PANEL + '%',
			height: '33.33%-1',
            pad: 1,
            getDataMethod: getStrategyTrade,
	        headers: ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Vol', 'AccountId']            
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
		const { processStatus, strategyData, posData, orderData, tradeData, pnlData } = t.globalData;
		t.strategyTable.refresh(strategyData, processStatus, pnlData)
		t.posTable.refresh(posData)
		t.orderTable.refresh(orderData, 'strategy');
		t.tradeTable.refresh(tradeData, 'strategy');
	}
	
	getData(){
		const  t = this;
		let timer = null;
		const runPromises = () => {
			clearTimeout(timer)
			const currentId = Object.keys(t.globalData.strategyData)[t.strategyTable.selectedIndex || 0];
			if(!currentId) return new Promise((resolve) => resolve());      
			//md + td
            const strategyListPromise = t.strategyTable.getData(t.globalData)
			.then(strategyData => t.globalData.strategyData = strategyData)
			.then(() => t.refresh())
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
			const tradingDay = buildTradingDay();
			const pnlDataPromise = getStrategysPnl(Object.keys(t.globalData.strategyData || {}), tradingDay).then(res => {
				let pnlData = {}
				res.forEach(p => pnlData[p.strategyId] = p.lastPnl)
				t.globalData.pnlData = pnlData;
			})

			return Promise.all([
				strategyListPromise, 
				posDataPromise,
				orderDataPromise,
				tradeDataPromise,
				pnlDataPromise
			]).finally(() => {
				timer = setTimeout(() => runPromises(), 3000)
			})
		}
		return runPromises()
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
		let boards = ['strategyTable', 'posTable', 'logTable', 'orderTable', 'tradeTable'];
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
	
	getProcessStatus(){
		const t = this;
		//循环获取processStatus
		var listProcessTimer;
		const startGetProcessStatus = () => {
			clearTimeout(listProcessTimer)
			return listProcessStatus()
			.then(res => t.globalData.processStatus = Object.freeze(res))
			.catch(err => console.error(err))
			.finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
		}
		return startGetProcessStatus()
	}

	getLogs(){
        const t = this;
		const currentId = Object.keys(t.globalData.strategyData)[t.strategyTable.selectedIndex || 0];  
		if(!currentId) return;      
        const logPath = path.join(LOG_DIR, `${currentId}.log`);  
        return getLog(logPath).then(({list}) => {
            list.forEach(l => {
				t.logTable.add(dealLog(l))
			})
		}).finally(() => t._watchCurrentProcessLog(currentId))
    }

	_watchCurrentProcessLog(processId){
		if(!processId) return;
		const t = this;
		if(t.logWatcher) t.logWatcher.unwatch();
		t.logWatcher = null;
		const logPath = path.join(LOG_DIR, `${processId}.log`);
		const watcher = new Tail(logPath);
		watcher.watch();
		watcher.on('line', line => {
			const logData = dealLogMessage(line);
			logData.forEach(l => t.logTable.add(dealLog(l)))
		})
		watcher.on('error', err => {
			watcher.unwatch();
			watcher = null;
		})
		t.logWatcher = watcher;
	}

	_afterSwitchStrategyProcess(index){
		const t = this;
		const strategyProcess = Object.values(t.globalData.strategyData || {})[index];
		switchStrategy(strategyProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
	}

	_afterSelected(){
		this.globalData = {
			...this.globalData,
			posData: {},
			tradeData: [],
			orderData: []
		};
		this.logTable.content = '';
		this.logTable.setScrollPerc(0)

		this.refresh();
		this.getLogs();
	}
}
	
export default () => {
	const strategyDashboard = new StrategyDashboard();
	strategyDashboard.init();
	strategyDashboard.bindEvent();
	strategyDashboard.render();
	strategyDashboard.getData().then(() => strategyDashboard.getLogs())
	// strategyDashboard.refresh();
	// strategyDashboard.getProcessStatus();   
	// setInterval(() => {
	// 	strategyDashboard.refresh();
	// }, 1000)	
}





