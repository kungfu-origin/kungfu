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

interface Pm2Packet {
  process: {
    pm_id: number;
  };
  data: {
    type: string;
    body: Record<string, string | number | boolean>;
  };
}

interface Pm2PacketMain {
  type: string;
  topic: string;
  data: object;
  id: number;
}

interface SwitchKfLocationPacketData {
  category: string;
  group: string;
  name: string;
  value: string;
  status: boolean;
}

class Pm2Bus {
  on(type: string, cb: (packet: Pm2Packet) => void);
}
