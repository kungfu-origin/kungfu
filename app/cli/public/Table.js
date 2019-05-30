import blessed from 'blessed';
import { calcuHeaderWidth, parseToString, TABLE_BASE_OPTIONS } from './utils';

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
	this.afterSelectMethod = options.afterSelectMethod || null;
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

	//bind event
	const t = this;
	box.on('focus', () => {
		if(!list.focused) list.focus()
	})
	list.on('focus', (e) => {
		process.nextTick(() => {
			const selected = e.selected;
			t.table.selectedIndex = selected;
			t.afterSelectMethod();
		})
		box.style.border.fg = 'blue'
	})
	list.on('blur', () => {
		box.style.border.fg = 'white'
	})

	list.on('keypress', (e, key) => {
		t.table.selectedIndex = t.table.selectedIndex || 0;
		if(key.name === 'up' && t.table.selectedIndex > 0) {
			process.nextTick(() => {
				t.table.selectedIndex = t.table.selectedIndex - 1;
				t.afterSelectMethod();
			})
		} 
		else if(key.name === 'down' ){
			const itemLen = t.table.children[1].ritems.length || 0;
			if(t.table.selectedIndex < itemLen - 1) {
				process.nextTick(() => {
					t.table.selectedIndex = t.table.selectedIndex + 1;
					t.afterSelectMethod();
				})
			}
		}
	})

	return box
} 


export default Table