import colors from 'colors';
import moment from 'moment';
import { toDecimal } from '__gUtils/busiUtils';
import { logger } from '__gUtils/logUtils';

String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
}

// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const offsetName: StringToStringObject = {
    '0': 'open',
    '1': 'close',
    '2': 'closeT',
    '3': 'closeY'
}

export const sideName: StringToStringObject = {
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
export const orderStatus: StringToStringObject = {
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
export const posDirection: StringToStringObject = {
    '0': 'Long',
    '1': 'Short' 
}

export const DEFAULT_PADDING: StringToNumberObject = {
    'top' : 1,
    'left' : 2,
    'right' : 1
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
		type: 'line',
		fg: 'grey'
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
export const parseToString = (targetList: any[], columnWidth: any[], pad = 2) => {
	return targetList.map((item: string, i: number) => {
		item = (item || '').toString();
		const r = /m([^]+)\u001b/
		//@ts-ignore
		const lw: any = item.match(r) === null ? item : item.match(r)[1];
		const len = lw.length;
		const colWidth: number | string = columnWidth[i] || 0;
		if(colWidth === 'auto') return item;
		const spaceLength = +colWidth - len;
		if(spaceLength < 0) return lw.slice(0, +colWidth)
		else if(spaceLength === 0) return item
		else return (item + new Array(spaceLength + 1).join(" "))
	}).join(new Array(pad + 2).join(" "))
}


export const calcuHeaderWidth = (target: string[], wish: any[]) => {
	wish = wish || [];
	return target.map((t: string, i) => {
		if(wish[i] === 'auto') return wish[i];
		if(t.length < (wish[i] || 0)) return wish[i]
		else return t.length
	})
}

export const parseAccountList = (target: any, accountList: Account[]) => {
	if(!target || !Object.keys(target || {}).length) target = {};
	//td
	accountList.forEach((item: Account) => {
		const accountId = item.account_id.toAccountId();
		if(target.accountData && target.accountData[accountId]) target.accountData[accountId] = {...target.accountData[accountId], ...item}
		else target.accountData[accountId] = {...item, status: '--', accumulated_pnl : '--', accumulated_pnl_ratio: '--', 'total': '--', 'avail': '--'}
	});
	//md
	accountList
	.filter((item: Account) => !!item.receive_md)
	.forEach((item: Account) => {
		const source = item.source_name;
		if(target.mdData[source]) target.mdData[source] = {...target.mdData[source], ...item}
		else target.mdData[source] = {...item, status: '--'}
	})
	return target
}

export const dealStatus = (status: string) => {
	status = status || '--'
	if(status === 'online') return colors.green(status);
	else if(status.indexOf('error') !== -1) return colors.red(status);
	else return status
}

export const dealNum = (num: number, percentage?: boolean) => {
	const percentageStr: string = percentage ? '%' : '';
	const targetNum: string = (percentageStr ? toDecimal(num, 4, 2) : toDecimal(num)) + '' || '--'
	if(targetNum === '--') return '--'
	if(+targetNum > 0) {
		return colors.red(targetNum + percentageStr).toString()
	}
	else if(+targetNum < 0) return colors.green(targetNum + percentageStr)
	else return targetNum + percentageStr
}

export const dealLog = (item: LogData) => {
	let type = item.type;
	if(type === 'error') type = colors.red(item.type);
	else if(type === 'warning') type = colors.yellow('warn');
	return parseToString(
		[`[${item.updateTime}]`, `${type}`, item.message], 
		[31, 5, 'auto'], 
		0
	)
}

export const buildTargetDateRange = () => {
	const momentDay = moment();
	const startDate = momentDay.add(-3, 'd').format('YYYYMMDD')
	const endDate = momentDay.add(2,'d').format('YYYYMMDD')
	return [startDate, endDate]
}

export const buildTradingDay = () => {
	return moment().format('YYYYMMDD')
}

export const parseSources = (accountSource: Sources): string[] => {
	return Object.values(accountSource).map((s: any) => `${s.source} (${s.typeName})`)
}

export const getStatus = (processId: string, processStatus: any) => {
	return processStatus[processId] === 'online'
}




