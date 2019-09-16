import blessed from 'blessed';
import { calcuHeaderWidth, parseToString, TABLE_BASE_OPTIONS } from '../utils';
import { logger } from '__gUtils/logUtils';

class Table {
	headers: string[];
	columnWidth: Array<any>;
	table: any;

	pad: number;
	constructor() {
		this.headers = [];
		this.columnWidth = [];
		this.table = null;
		this.getDataMethod = null;
		this.afterSelectMethod = null;
		this.pad = 2;
	}


	build(options: any) {
		options = {
			...options,
			headers: options.headers || this.headers,      
			columnWidth: options.columnWidth || this.columnWidth
		}
		// if(!options.headers) throw new Error('Options must include attribute: headers')
		// if(!options.getDataMethod) throw new Error(`${options.label} getDataMethod is required!`)
		// this.getDataMethod = options.getDataMethod;
		// this.afterSelectMethod = options.afterSelectMethod || function(){};
		// this.afterSwitchMethod = options.afterSwitchMethod || function(){};
		if(options.pad !== undefined) this.pad = options.pad;
		if(options.headers !== undefined) this.headers = options.headers;
		if(options.columnWidth !== undefined) this.columnWidth = options.columnWidth;
		this.table = this.init(options)
		// this.table.getData = this.getData.bind(this);
		// this.table.refresh = this.refresh.bind(this);
		// this.table.selectedIndex = 0;
		return this.table;
	}

	init(options: any){
		const headers = options.headers || this.headers;
		const columnWidth = options.columnWidth || this.columnWidth;
		const pad = options.pad || this.pad;
		const content = ' ' + parseToString(headers, calcuHeaderWidth(headers, columnWidth), pad)
		const box = blessed.box({
			...options,
			content: content,
			border: {
				type: 'line'
			},
			interactive: false,
			mouse: false,
			style: {
				fg: 'grey'
			},
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
			height: '100%-3',
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
	
		list.on('focus', (e) => {
			box.style.border.fg = 'blue'
			process.nextTick(() => {
				// t.afterSelected(list)
			})
		})
	
		list.on('blur', () => {
			box.style.border.fg = 'white'
		})
	
		list.key(['enter'], () => {
			// t.afterSwitchMethod(list.selected);
			// t.afterSelected(list)		
		})
	
		list.key(['down', 'up'], () => {
			// t.afterSelected(list)
		})
	}	
}


// Table.prototype.getData = function(){}

// Table.prototype.refresh = function(){}



// Table.prototype.afterSelected = function(target){
// 	const t = this;
// 	const newIndex = target.selected;
// 	if(newIndex !== t.table.selectedIndex) {
// 		t.table.selectedIndex = newIndex;
// 		t.afterSelectMethod(t.table.selectedIndex);
// 	}
// }


export default Table