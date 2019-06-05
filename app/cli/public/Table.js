import blessed from 'blessed';
import { calcuHeaderWidth, parseToString, TABLE_BASE_OPTIONS } from './utils';
import { logger } from '__gUtils/logUtils';

function Table(){
	this.headers = [];
	this.columnWidth = [];
    this.table = null;
	this.getDataMethod = null;
	this.afterSelectMethod = null;
}

Table.prototype.build = function(options) {
	if(!options.getDataMethod) throw new Error(`${options.label} getDataMethod is required!`)
	this.getDataMethod = options.getDataMethod;
	this.afterSelectMethod = options.afterSelectMethod || function(){};
	this.afterSwitchMethod = options.afterSwitchMethod || function(){};
    this.table = this.init({
        ...options,
		headers: this.headers,      
		columnWidth: this.columnWidth
    })
    this.table.getData = this.getData.bind(this);
	this.table.refresh = this.refresh.bind(this);
	this.table.selectedIndex = 0;
    return this.table;
}

Table.prototype.getData = function(){}

Table.prototype.refresh = function(){}

Table.prototype.init = function(options){
    if(!options.headers) throw new Error('options must include attribute: headers')
	const headers = options.headers || [];
	const columnWidth = options.columnWidth || [];
	const content = ' ' + parseToString(headers, calcuHeaderWidth(headers, columnWidth))
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
	this.bindEvent(box, list)
	return box
} 

Table.prototype.bindEvent = function(box, list){
	//bind event
	const t = this;
	box.on('focus', () => {
		if(!list.focused) list.focus()
	})
	list.on('focus', (e) => {
		box.style.border.fg = 'blue'
	})
	list.on('blur', () => {
		box.style.border.fg = 'white'
	})
	list.on('select', () => {
		const newIndex = list.selected;
		if(newIndex !== t.table.selectedIndex) {
			t.table.selectedIndex = newIndex;
			t.afterSelectMethod(t.table.selectedIndex);
		}
	})
	list.key(['enter'], () => {
		const newIndex = list.selected;
		t.afterSwitchMethod(t.table.selectedIndex);
		if(newIndex !== t.table.selectedIndex) {
			t.table.selectedIndex = newIndex;
			t.afterSelectMethod(t.table.selectedIndex);
		}
	})
}


export default Table