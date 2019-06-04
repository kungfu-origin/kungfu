import colors from 'colors';
import { startTd, startMd, startStrategy, startMaster, deleteProcess } from '__gUtils/processUtils.js';
import { setTasksDB } from '@/io/base';

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
    0: 'L',
    1: 'S' 
}

/**
 * @param  {Array} targetList
 * @param  {} columnWidth
 * @param  {} pad=2
 */
export const parseToString = (targetList, columnWidth=[], pad=2) => {
	return targetList.map((l, i) => {
		l = l + '';
		const r = /m([^%]+)\u001b/
		const lw = l.match(r) === null ? l : l.match(r)[1];
		const len = lw.length;
		const colWidth = columnWidth[i];
		if(colWidth === undefined) return l;
		const spaceLength = colWidth - len;
		if(spaceLength < 0) return lw.slice(0, colWidth)
		else if(spaceLength === 0) return l
		else return (l + new Array(spaceLength + 1).join(" "))
	}).join(new Array(pad + 2).join(" "))
}


export const calcuHeaderWidth = (target, wish=[]) => {
	return target.map((t, i) => {
		if(t.length < (wish[i] || 0)) return wish[i]
		else return t.length
	})
}

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
	interactive: true,
	keys: true,
	align: 'left',
	autoCommandKeys: true,
	tags: true,
	mouse: true,
	rows: [],
	items: [],
	style: {
		fg: 'white',
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

export const switchMaster = (globalStatus) => {
	if(globalStatus['master'] !== 'online') return startMaster().catch(err => {})
	else return deleteProcess('master').catch(err => {})
}

export const switchTd = (processData, globalStatus) => {
	const {account_id, source_name, config} = processData;
	const processId = `td_${account_id}`;
	if(globalStatus[processId] === 'online') return deleteProcess(processId);
	return setTasksDB({name: processId, type: 'md', config})
    .then(() => startTd(source_name, processId)) //开启td,pm2
}

export const switchMd = (processData, globalStatus) => {
	const {source_name, config} = processData;
	const processId = `md_${source_name}`;
	if(globalStatus[processId] === 'online') return deleteProcess(processId);
	return setTasksDB({name: processId, type: 'td', config})
	.then(() => startMd(source_name, processId)) //开启td,pm2
}

export const switchStrategy = (processData, globalStatus) => {
	const processId = processData.strategy_id;
	if(globalStatus[processId] === 'online') return deleteProcess(processId);
	const strategyPath = processData.strategyPath || '';
	const config = JSON.stringify({
		strategy_id: processId,
		strategy_path: strategyPath
	})
	const postData = {
		name: processId, 
		type: 'strategy',
		config
	}
	return setTasksDB(postData)
	.then(() => startStrategy(processId, strategyPath))// 启动策略
}