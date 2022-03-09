import { Widgets } from 'blessed';

export const DEFAULT_PADDING: Record<string, number> = {
  top: 1,
  left: 2,
  right: 1,
};

export const TABLE_BASE_OPTIONS: Widgets.ElementOptions = {
  content: '',
  padding: 0,
  scrollable: true,
  scrollbar: {
    ch: ' ',
    inverse: true,
  },
  border: {
    type: 'line',
    // fg: 'white',
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
      },
    },
    item: {
      border: {
        fg: 'white',
      },
    },
    scrollbar: {
      bg: 'blue',
      fg: 'black',
    },
    selected: {
      bold: true,
    },
  },
};
