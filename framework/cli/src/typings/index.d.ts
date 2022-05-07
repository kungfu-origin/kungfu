import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { Widgets } from 'blessed';
import { ProcessDescription } from 'pm2';

interface PromptAnswer {
  source: string;
}

type PromptInputType =
  | 'input'
  | 'number'
  | 'confirm'
  | 'list'
  | 'rawlist'
  | 'expand'
  | 'checkbox'
  | 'password'
  | 'editor'
  | 'path';

interface PromptQuestion {
  type: PromptInputType;
  name: string;
  default?: KungfuApi.KfConfigValue;
  choices: (string | number)[];
  message: string;
  validate: (value: KungfuApi.KfConfigValue) => Promise<Error | boolean>;
}

interface ProcessListItem {
  processId: string;
  processName: string;
  typeName: string;
  category: KfCategoryTypes;
  group: string;
  name: string;
  value: record<string, KungfuApi.KfConfigValue>;
  statusName: string;
  status: string;
  monit: ProcessDescription['monit'];
}

class ListElementResolved extends Widgets.ListElement {
  setItems(items: string[]): void;
  selected: number;
  focused: boolean;
}

interface SwitchKfLocationPacketData {
  category: string;
  group: string;
  name: string;
  value: string;
  status: boolean;
}

declare module NodeJS {
  interface console {
    success: (str: string) => void;
  }
}

interface TableOption {
  headers?: string[];
  columnWidth?: (number | string)[];
  label?: string;
  parent?: Widgets.Screen;
  top?: string;
  left?: string;
  width?: string;
  height?: string;
  pad?: number;
  cell?: Widgets.StyleListTable['cell'];
  style?: {
    cell: Widgets.StyleListTable['cell'];
  };
}

interface BoxElementResolved extends Widgets.BoxElement {
  setItems(list: string[]): void;
  headers: string[];
  childList: ListElementResolved;
}

declare global {
  interface Console {
    success: (str: string) => void;
  }
}
