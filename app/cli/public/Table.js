import blessed from 'blessed';
import { calcuHeaderWidth, parseToString, TABLE_BASE_OPTIONS } from './utils';

function Table(){
    this.headers = []
    this.table = null;
	this.getDataMethod = null;
}

Table.prototype.build = function(options) {
	if(!options.getDataMethod) throw new Error(`${options.label} getDataMethod is required!`)
    this.getDataMethod = options.getDataMethod;
    this.table = this.init({
        ...options,
        headers: this.headers,      
        style: {
			cell: {
				selected: {
					bold: true,
					bg: 'blue',
					fg: 'white'
				},
			},
		}
    })
    this.table.getData = this.getData.bind(this);
    this.table.refresh = this.refresh.bind(this);
    return this.table;
}

Table.prototype.getData = function(){}

Table.prototype.refresh = function(){}

Table.prototype.init = (options) => {
    if(!options.headers) throw new Error('options must include attribute: headers')
	const headers = options.headers || [];
	const content = ' ' + parseToString(headers, calcuHeaderWidth(headers))
	const box = blessed.box({
		...options,
		content: content,
		border: {
			type: 'line'
		},
		mouse: true,
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

	box.on('focus', () => {
		if(!list.focused) list.focus()
		box.style.border.fg = TABLE_BASE_OPTIONS.style.focus.border.fg
	})
	list.on('blur', () => {
		box.style.border.fg = TABLE_BASE_OPTIONS.style.item.border.fg
	})
	list.on('focus', () => {
		box.style.border.fg = TABLE_BASE_OPTIONS.style.focus.border.fg
	})

	return box
} 


export default Table