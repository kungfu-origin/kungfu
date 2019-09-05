import colors from 'colors';
import moment from 'moment';
import { startTd, startMd, startStrategy, startMaster, deleteProcess } from '__gUtils/processUtils';
import { setTasksDB } from '__io/db/base';
import { toDecimal } from '__gUtils/busiUtils';
import { logger } from '__gUtils/logUtils';

String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
}

// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const offsetName = {
    '0': 'open',
    '1': 'close',
    '2': 'closeT',
    '3': 'closeY'
}

export const sideName = {
    '0': 'buy',
    '1': 'sell'
}

// Unknown = '0', // 未知
// Submitted = '1', //已提交 
// Pending = '2', // 等待
// Cancelled = '3', // 已撤销
// Error = '4', // 错误
// Filled = '5', //已成交
// PartialFilledNotActive = '6', //部分撤单
// PartialFilledActive = '7' //正在交易
// 3,4,5,6已完成
export const orderStatus = {
    '0': 'wait',
    '1': 'Submitted',
    '2': 'Pending',
    '3': 'Cancelled',
    '4': 'Error',
    '5': 'Filled',
    '6': 'ParticalFilled',
    '7': 'trading',
}

//     DirectionLong = '0'
//    DirectionShort = '1'
export const posDirection = {
    0: 'Long',
    1: 'Short' 
}

export const DEFAULT_PADDING = {
    top : 1,
    left : 1,
    right : 1
};

export const TABLE_BASE_OPTIONS = {
	content: '',
	padding: 0,
	scrollable: true,
	scrollbar: {
		ch: ' ',
		inverse: true
	},
	border: {
		type: 'line'
	},
	keys: true,
	align: 'left',
	autoCommandKeys: true,
	tags: true,
	mouse: true,
	clickable: true,
	interactive: true,
	rows: [],
	items: [],
	style: {
		focus: {
			border: {
				fg: 'blue',
			}
		},
		item: {
			border: {
				fg: 'white',
			}
		},
		scrollbar: {
			bg: 'blue',
			fg: 'black'
		},
		selected: {
			bold: true,
		},
	}
}


/**
 * @param  {Array} targetList
 * @param  {} columnWidth
 * @param  {} pad=2
 */
export const parseToString = (targetList, columnWidth=[], pad=2, debug) => {
	return targetList.map((l, i) => {
		l = (l || '').toString();
		const r = /m([^]+)\u001b/
		const lw = l.match(r) === null ? l : l.match(r)[1];
		const len = lw.length;
		const colWidth = columnWidth[i] || 0;
		if(colWidth === 'auto') return l;
		const spaceLength = colWidth - len;
		if(spaceLength < 0) return lw.slice(0, colWidth)
		else if(spaceLength === 0) return l
		else return (l + new Array(spaceLength + 1).join(" "))
	}).join(new Array(pad + 2).join(" "))
}


export const calcuHeaderWidth = (target, wish=[]) => {
	return target.map((t, i) => {
		if(wish[i] === 'auto') return wish[i];
		if(t.length < (wish[i] || 0)) return wish[i]
		else return t.length
	})
}

export const parseAccountList = (target={}, accountList) => {
	//td
	accountList.forEach(l => {
		const accountId = l.account_id.toAccountId();
		if(target.accountData[accountId]) target.accountData[accountId] = {...target.accountData[accountId], ...l}
		else target.accountData[accountId] = {...l, status: '--', accumulated_pnl : '--', accumulated_pnl_ratio: '--', 'total': '--', 'avail': '--'}
	});
	//md
	accountList.filter(l => !!l.receive_md).forEach(l => {
		const source = l.source_name;
		if(target.mdData[source]) target.mdData[source] = {...target.mdData[source], ...l}
		else target.mdData[source] = {...l, status: '--'}
	})
	return target
}

export const dealStatus = (status) => {
	status = status || '--'
	if(status === 'online') return colors.green(status);
	else if(status === 'error') return colors.red(status);
	else return colors.grey(status)
}

export const dealNum = (num, percentage) => {
	percentage = percentage ? '%' : '';
	num = (percentage ? toDecimal(num, 4, 2) : toDecimal(num)) + '' || '--'
	if(num === '--') return '--'
	if(+num > 0) {
		return colors.red(num + percentage).toString()
	}
	else if(+num < 0) return colors.green(num + percentage)
	else return num + percentage
}

export const dealPnlData = (data) => {
	let xAxisData = []
	let serirsData = []
	data && data.map(item => {
		const time = item.trading_day
		xAxisData.push(time.slice(4))
		serirsData.push(toDecimal(item.accumulated_pnl_ratio, 4, 2))
	})
	return {x: xAxisData, y:serirsData}
}

export const dealLog = (l) => {
	let type = l.type;
	if(type === 'error') type = colors.red(l.type);
	else if(type === 'warning') type = colors.yellow('warn');
	return parseToString([`[${l.updateTime}]`, `${type}`, l.message], [31, 5, 'auto'], 0)
}

export const buildDateRange = () => {
	const momentDay = moment();
	const startDate =momentDay.format('YYYY-MM-DD')
	const endDate = momentDay.add(1,'d').format('YYYY-MM-DD')
	return [startDate, endDate]
}

export const buildTradingDay = () => {
	return moment().format('YYYYMMDD')
}

export const parseSources = (sourcesData) => {
    return Object.values(sourcesData).map(s => `${s.source} (${s.typeName})`)
}

export const getStatus = (processId, processStatus) => {
	return processStatus[processId] === 'online'
}
