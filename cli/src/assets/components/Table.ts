import { calcuHeaderWidth, parseToString, TABLE_BASE_OPTIONS } from '@/assets/scripts/utils';
import { logger } from '__gUtils/logUtils';

const blessed = require('blessed');
const colors = require('colors');

class Table {
	headers: string[];
	columnWidth: Array<any>;
	table: any;

	pad: number;
	constructor() {
		this.headers = [];
		this.columnWidth = [];
		this.table = null;
		this.pad = 2;
	}


	build(options: any) {
		options = {
			...options,
			headers: options.headers || this.headers,      
			columnWidth: options.columnWidth || this.columnWidth
		}
		if(options.pad !== undefined) this.pad = options.pad;
		if(options.headers !== undefined) this.headers = options.headers;
		if(options.columnWidth !== undefined) this.columnWidth = options.columnWidth;
		this.table = this.init(options)
		return this.table;
	}

	focus() {
		this.table.focus()
	}

	init(options: any){
		const headers = options.headers || this.headers;
		const columnWidth = options.columnWidth || this.columnWidth;
		const pad = options.pad || this.pad;
		const content = ' ' + parseToString(headers, calcuHeaderWidth(headers, columnWidth), pad)
		const box = blessed.box({
			...options,
			content: colors.bold(content),
			border: {
				type: 'line',
				fg: 'grey'
			},
			interactive: false,
			mouse: false,
			style: {
				fg: 'grey'
			},
			padding: {
				top: 1
			},
			tabSize: 10
		});
		const list = blessed.list({
			...TABLE_BASE_OPTIONS,
			...options.cell,
			label: '',
			content: '',
			parent: box,
			top: 1,
			border: {
				transparent: true
			},
			height: '100%-4',
			items: [],
			style: {
				...TABLE_BASE_OPTIONS.style,
				...((options.style || {}).cell || {}),
			},
		});

		box.setItems = list.setItems.bind(list);
		box.headers = headers;
		box.childList = list;
		this.bindEvent(box, list)
		return box
	} 

	bindEvent(box: any, list: any){
		//bind event
		const t = this;
		box.on('focus', () => {
			if(!list.focused) list.focus()
		})
	
		list.on('focus', () => {
			box.style.border.fg = 'blue'
		})
	
		list.on('blur', () => {
			box.style.border.fg = 'grey'
		})
	}	
}



export default Table