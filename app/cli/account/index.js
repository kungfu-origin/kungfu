const blessed = require('blessed');
const contrib = require('blessed-contrib');
const moment = require('moment');
import { offsetName, orderStatus, sideName, posDirection } from "../public/utils";
import { getAccountList, getAccountPos, getAccountOrder } from '@/io/account.js';
const colors = require('colors')

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;

//定义Dashboard
function Dashboard(){
	const t = this;
	t.screen = blessed.screen({
		smartCSR: true,
		fullUnicode: true
	});

	t.screen.title = 'Account Dashboard';
	t.accountList = null;
	t.posList = null;
	t.mdList = null;
	t.pnl = null;
	t.tradeList = null;
	t.orderList = null;
	t.boxInfo = null;

	t.accountListHeaders = [' Account', ' Source', ' State', ' Accum',' AccumRt',' Total', ' Avail'];
	t.mdListHeaders = [' Source', ' Account', ' State'];
	t.posListHeaders = [' Ticker', ' Dir', ' Yest', ' Total', ' UnRealPnl']
	t.orderListHeaders = [' OrderTime', ' Ticker', ' Side', ' Offset', ' Price', ' Filled/Not', ' State', ' Str']
	t.tradeListHeaders = [' UpdateTime', ' Ticker', ' Side', ' Offset', ' Price', ' Vol', ' Strat']

	t.globalData = {
		accountData: {},
		posData: [],
		mdData: {},
		pnlData: [],
		tradeData: [],
		orderData: []
	};

	t.currentAccount = '15040900'

}

Dashboard.prototype.init = function(){
	const t = this;
	t.initAccountList();
	t.initMdList();
	t.initOrderList();
	t.initPnlList();
	t.initTradeList();
	t.initPosList();
	t.initBoxInfo();
}

Dashboard.prototype.initAccountList = function(){
	const t = this;
	t.accountList = contrib.table({
		label: ' Trading Engines ',
		top: '0',
		left: '0',
		width: WIDTH_LEFT_PANEL + '%',
		height: '33.33%',
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
		fg: 'white',
		style: {
			selected: {
				bg: 'blue',
				fg: 'white'
			},
			scrollbar: {
				bg: 'blue',
				fg: 'black'
			},
			fg: 'white',
			border: {
				fg: 'blue'
			},
			header: {
				fg: 'blue'
			}
		},
		columnSpacing: 5, //in chars
		columnWidth: t.calcuHeaderWidth(t.accountListHeaders, [8]),
		data: {
			headers: t.accountListHeaders, 
			data:[]
		}
	});



	t.screen.append(t.accountList);
	t.accountList.focus();
}

Dashboard.prototype.initMdList = function(){
	const t = this;
	t.mdList = contrib.table({
		label: ' Market Engines ',
		top: '33.33%',
		left: '0',
		width: WIDTH_LEFT_PANEL / 2 + '%',
		height: '23.66%',
		padding: 0,
		align: 'left',
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
		fg: 'white',
		interactive: false,
		style: {
			fg: 'white',
			border: {
			fg: 'white'
			},
			scrollbar: {
			bg: 'blue',
			fg: 'black'
			}
		},
		columnSpacing: 5, //in chars
		columnWidth: t.calcuHeaderWidth(t.mdListHeaders, [0, 10]),
		data: {
			headers: t.mdListHeaders, 
			data:[]
		}
	});

	t.screen.append(t.mdList);
}

Dashboard.prototype.initPosList = function(){
	const t = this;
	t.posList = contrib.table({
		label: ' Positions ',
		top: '0',
		left: WIDTH_LEFT_PANEL + '%',
		width: 100 - WIDTH_LEFT_PANEL + '%',
		height: '56%',
		padding: 0,
		scrollable: true,
		scrollbar: {
			ch: ' ',
			inverse: false
		},
		keys: true,
		fg: 'white',
		interactive: false,
		autoCommandKeys: true,
		tags: true,
		border: {
			type: 'line'
		},
		style: {
			fg: 'white',
			border: {
				fg: 'white'
			},
			scrollbar: {
				bg: 'blue',
				fg: 'black'
			}
		},
		columnSpacing: 4, //in chars
		columnWidth: t.calcuHeaderWidth(t.posListHeaders, [0, 4, 6, 6, 10]),
		data: {
			headers: t.posListHeaders, 
			data:[]
		}
	});
	t.screen.append(t.posList);
}

Dashboard.prototype.initPnlList = function(){
	const t = this;
	t.pnl = blessed.list({
		label: ' Pnl ',
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
			fg: 'white',
			border: {
			fg: 'white'
			},
			scrollbar: {
			bg: 'blue',
			fg: 'black'
			}
		}
	});
	t.screen.append(t.pnl);
}

Dashboard.prototype.initOrderList = function(){
	const t = this;
	t.orderList = contrib.table({
		label: ' Current Orders ',
		top: '56%',
		width: WIDTH_LEFT_PANEL - 5 + '%',
		height: '40%',
		scrollable: true,
		scrollbar: {
			ch: ' ',
			inverse: false
		},
		keys: true,
		autoCommandKeys: true,
		tags: true,
		interactive: false,
		fg: 'white',
		border: {
			type: 'line'
		},
		style: {
			fg: 'white',
			border: {
			fg: 'white'
			},
			scrollbar: {
			bg: 'blue',
			fg: 'black'
			}
		},
		columnSpacing: 2, //in chars
		columnWidth: t.calcuHeaderWidth(t.orderListHeaders, [0,0,0,0,0,0,8]),
		data: {
			headers: t.orderListHeaders, 
			data:[]
		}
	});
	t.screen.append(t.orderList);

}

Dashboard.prototype.initTradeList = function(){
	const t = this;
	t.tradeList = contrib.table({
		label: ' Today Trades ',
		top: '56%',
		left: WIDTH_LEFT_PANEL - 5 + '%',
		width: 100 - WIDTH_LEFT_PANEL + 5 + '%',
		height: '40%',
		scrollbar: {
			ch: ' ',
			inverse: false
		},
		keys: true,
		autoCommandKeys: true,
		tags: true,
		border: {
			type: 'line'
		},
		style: {
			fg: 'white',
			border: {
			fg: 'white'
			},
			scrollbar: {
			bg: 'blue',
			fg: 'black'
			}
		},
		columnSpacing: 2, //in chars
		columnWidth: t.calcuHeaderWidth(t.tradeListHeaders, []),
		data: {
			headers: t.tradeListHeaders, 
			data:[]
		}
	});

	t.screen.append(t.tradeList);
}

Dashboard.prototype.initBoxInfo = function() {
	const t = this;
	t.boxInfo = blessed.text({
		content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit{|} {cyan-fg}{bold}',
		left: '0%',
		top: '95%',
		width: '100%',
		height: '6%',
		valign: 'middle',
		tags: true,
		style: {
			fg: 'white'
		}
	});
	
	t.screen.append(t.boxInfo);
}

Dashboard.prototype.bindEvent = function(){
	const t = this;
	let i = 0;
	let boards = ['accountList', 'posList', 'mdList', 'pnl', 'tradeList', 'orderList'];
	t.screen.key(['left', 'right'], (ch, key) => {
		(key.name === 'left') ? i-- : i++;
		if (i === 6) i = 0;
		if (i === -1) i = 5;
		t[boards[i]].focus();
		t[boards[i]].style.border.fg = 'blue';
		if (key.name === 'left') {
			if (i == 5) t[boards[0]].style.border.fg = 'white';
			else t[boards[i + 1]].style.border.fg = 'white';
		} else {
			if (i == 0) t[boards[5]].style.border.fg = 'white';
			else t[boards[i - 1]].style.border.fg = 'white';
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

Dashboard.prototype.fresh = function(){
	const t = this;

	const accountData = Object.values(t.globalData.accountData || {});
	t.accountList.setData({
		headers: t.accountListHeaders, 
		data: accountData.map(a => [a.account_id.toAccountId(), a.source_name, a.status, a.accumulated_pnl, a.accumulated_pnl_ratio, a.total, a.avail])
	});

	const mdData = Object.values(t.globalData.mdData || {})
	t.mdList.setData({
		headers: t.mdListHeaders,
		data: mdData.map(m => [m.source_name, m.account_id.toAccountId(), m.status])
	});

	const posData = Object.values(t.globalData.posData || {})
	t.posList.setData({
		headers: t.posListHeaders, 
		data: posData.map(p => {
			let direction = posDirection[p.direction]
			if(direction === 'L') direction = colors.red(direction);
			else direction = colors.green(direction);

			let unRealizedPnl = new Number(p.unrealized_pnl).toFixed(2);
			if(unRealizedPnl > 0) unRealizedPnl = colors.red(unRealizedPnl)
			else if(unRealizedPnl < 0) unRealizedPnl = colors.green(unRealizedPnl)
			else unRealizedPnl = colors.grey(unRealizedPnl)
			return [
				p.instrument_id, 
				direction,
				p.yesterday_volume, 
				p.volume, 
				unRealizedPnl
			]
		})
	});
	
	const orderData = t.globalData.orderData
	t.orderList.setData({
		headers: t.orderListHeaders,
		data: orderData.map(o => {
			let side = sideName[o.side] ? sideName[o.side] : '--';
			if(side === 'buy') side = colors.red(side);
			else if(side === 'sell') side = colors.green(side);

			let offset = offsetName[o.offset] ? offsetName[o.offset] : '--';
			if(offset === 'open') offset = colors.red(offset);
			else if(offset.indexOf('close') !== -1) offset = colors.green(offset);

			let status = orderStatus[o.status]
			if([3, 5, 6].indexOf(o.status) !== -1) status = colors.green(status);
			else if(+o.status === 4) status = colors.red(status);
			else status = colors.grey(status);

			return [
				o.insert_time && moment(o.insert_time/1000000).format("HH:mm:ss"),
				o.instrument_id,
				side,
				offset,
				o.limit_price,
				o.volume_traded + '/' + o.volume_left,
				status,
				o.client_id
			]
		}),
	});

	// t.tradeList.setData({
	// 	// headers: ['Account', 'Source', 'status'], 
	// 	data: t.globalData.tradeData
	// });
}

Dashboard.prototype.getData = function(){
	const  t = this;
	const getAccountListPromise = getAccountList().then(list => {
		//td
		list.forEach(l => {
			const accountId = l.account_id.toAccountId();
			if(t.globalData.accountData[accountId]) t.globalData.accountData[accountId] = {...t.globalData.ccountData[accountId], ...l}
			else t.globalData.accountData[accountId] = {...l, status: '--', accumulated_pnl : '--', accumulated_pnl_ratio: '--', 'total': '--', 'avail': '--'}
		});

		//md
		list.filter(l => !!l.receive_md).forEach(l => {
			const source = l.source_name;
			if(t.globalData.mdData[source]) t.globalData.mdData[source] = {...t.globalData.mdData[source], ...l}
			else t.globalData.mdData[source] = {...l, status: '--'}
		})
	})

	//pos
	const getPosPromise = getAccountPos(t.currentAccount, {}).then(pos => {
		t.globalData.posData = pos || {}
	})

	//order
	const getOrderPromise = getAccountOrder(t.currentAccount, {}).then(orders => {
		t.globalData.orderData = orders || {}

	})

	Promise.all([getAccountListPromise, getPosPromise, getOrderPromise]).then(() => {
		t.fresh()
	})
}

Dashboard.prototype.calcuHeaderWidth = (target, wish) => {
	return target.map((t, i) => {
		if(t.length < (wish[i] || 0)) return wish[i]
		else return t.length
	})
}


const accountDashboard = new Dashboard();
accountDashboard.init();
accountDashboard.bindEvent();
accountDashboard.render();
accountDashboard.getData();
