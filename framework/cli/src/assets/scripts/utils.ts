import { toDecimal } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { ProcessStatusConfig } from '@kungfu-trader/kungfu-uicc/config/tradingConfig';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';

const colors = require('colors')

String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
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
		fg: 'white'
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
export const parseToString = (targetList: any[], columnWidth: any[], pad=2) => {
	return targetList.map((item: string, i: number) => {

		if (item + '' === '0') item = '0';
		item = (item || '').toString();

		const lw = item
		.replace(/\u001b\[1m/g, '')
		.replace(/\u001b\[22m/g, '')
		.replace(/\u001b\[31m/g, '')
		.replace(/\u001b\[32m/g, '')
		.replace(/\u001b\[33m/g, '')		
		.replace(/\u001b\[34m/g, '')
		.replace(/\u001b\[35m/g, '')
		.replace(/\u001b\[36m/g, '')
		.replace(/\u001b\[37m/g, '')
		.replace(/\u001b\[38m/g, '')
		.replace(/\u001b\[39m/g, '')
		.replace(/\u001b\[45m/g, '')
		.replace(/\u001b\[49m/g, '')
		// console.log(lw.match(''), lw)

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

export const dealStatus = (status: string | number) => {
	if (status === '--') return status;
	if (!ProcessStatusConfig[status]) return status;
	const name: string = ProcessStatusConfig[status].name || '';
	const level: number = ProcessStatusConfig[status].level || 0;
	if(level == 1) return colors.green(name);
	else if(level == 0) return colors.white(name);
	else if (level == -1) return colors.red(name);
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

export const parseSources = (accountSource: Sources): string[] => {
	return Object.values(accountSource).map((s: any) => `${s.source} (${s.typeName})`)
}

export const getStatus = (processId: string, processStatus: any) => {
	return processStatus[processId] === 'online'
}


export const getQuestionInputType = (originType: string) => {
    switch(originType) {
        case 'str':
            return 'input';
        case 'int':
            return 'number';
        case 'float':
            return 'number';
        case 'select':
			return 'list';
		case 'sources':
            return 'list';
        case 'bool':
            return 'confirm';
        case 'file':
			return 'path';
        default:
            return 'input'
    }
}

export const renderSelect = (configItem: AccountSettingItem) => {
    if(configItem.type === 'select') return `(${(configItem.data || []).map(item => item.value || "").join('|')})`
    else return ''
}

export const getKungfuTypeFromString = (typeString: string) => {
    const isTd = typeString.toLocaleLowerCase().includes('td')
    const isMd = typeString.toLocaleLowerCase().includes('md')
    const isStrategy = typeString.toLocaleLowerCase().includes('strategy')
    
    if(isTd) return 'td';
    else if(isMd) return 'md';
    else if(isStrategy) return 'strategy'
    else return ''
}