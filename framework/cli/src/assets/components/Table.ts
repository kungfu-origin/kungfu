import { TABLE_BASE_OPTIONS } from '../config';
import { calcHeaderWidth, parseToString } from '../methods/utils';
import blessed, { Widgets } from 'blessed';
import colors from 'colors';
import {
  BoxElementResolved,
  ListElementResolved,
  TableOption,
} from 'src/typings';

class Table {
  headers: string[];
  columnWidth: (string | number)[];
  table: BoxElementResolved | null;

  pad: number;
  constructor() {
    this.headers = [];
    this.columnWidth = [];
    this.table = null;
    this.pad = 2;
  }

  build(options: TableOption) {
    options = {
      ...options,
      headers: options.headers || this.headers,
      columnWidth: options.columnWidth || this.columnWidth,
    };
    if (options.pad !== undefined) this.pad = options.pad;
    if (options.headers !== undefined) this.headers = options.headers;
    if (options.columnWidth !== undefined)
      this.columnWidth = options.columnWidth;
    this.table = this.init(options);
    return this.table;
  }

  focus() {
    this.table.focus();
  }

  init(options: TableOption): BoxElementResolved {
    const headers = options.headers || this.headers;
    const columnWidth = options.columnWidth || this.columnWidth;
    const pad = options.pad || this.pad;
    const content =
      ' ' + parseToString(headers, calcHeaderWidth(headers, columnWidth), pad);

    const box = blessed.box({
      ...options,
      content: colors.bold(content),
      border: {
        type: 'line',
        // fg: 'white',
      },
      interactive: false,
      mouse: false,
      style: {
        fg: 'white',
      },
      padding: {
        top: 1,
      },
      tabSize: 10,
    }) as BoxElementResolved;

    const list = blessed.list({
      ...TABLE_BASE_OPTIONS,
      ...options.cell,
      label: '',
      content: '',
      parent: box,
      top: 1,
      border: {
        transparent: true,
      },
      height: '100%-4',
      items: [],
      style: {
        ...TABLE_BASE_OPTIONS.style,
        ...((options.style || {}).cell || {}),
      },
    }) as unknown as ListElementResolved;

    box.setItems = list.setItems.bind(list);
    box.headers = headers;
    box.childList = list;
    this.bindEvent(box, list);
    return box;
  }

  bindEvent(box: Widgets.BoxElement, list: ListElementResolved) {
    box.on('focus', () => {
      box.style.border.fg = 'blue';
      if (!list.focused) list.focus();
    });
    list.on('focus', () => {
      box.style.border.fg = 'blue';
    });

    list.on('blur', () => {
      box.style.border.fg = 'white';
    });
  }
}

export default Table;
