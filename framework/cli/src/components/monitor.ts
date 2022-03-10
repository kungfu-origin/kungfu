import Dashboard from '../assets/components/Dashboard';
import MessageBox from '../assets/components/MessageBox';
import Loading from '../assets/components/Loading';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS } from '../assets/config/index';
import blessed, { Widgets } from 'blessed';
import { ListElementResolved, ProcessListItem } from '../typings';
import {
  processListObservable,
  switchProcess,
} from '../assets/actions/processList';
import { dealMemory, parseToString } from '../assets/methods/utils';
import { Log } from '../assets/actions/log';
import {
  debounce,
  setTimerPromiseTask,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const WIDTH_LEFT_PANEL = 35;

export class MonitorDashboard extends Dashboard {
  boards: {
    processBoard: ListElementResolved | null;
    logBoard: ListElementResolved | null;
    message: Widgets.MessageElement | null;
    loader: Widgets.LoadingElement | null;
    boxInfo: Widgets.TextElement | null;
  };

  processList: ProcessListItem[];
  log: Log | null;

  constructor() {
    super();
    this.screen.title = 'Processes Dashboard';

    this.boards = {
      processBoard: null,
      logBoard: null,
      message: null,
      loader: null,
      boxInfo: null,
    };

    this.processList = [];
    this.log = null;
    this.init();
  }

  init() {
    this.initProcessList();
    this.initLog();
    this.initBoxInfo();
    this.initMessage();
    this.initLoader();
    this.screen.render();

    this.bindEvent();
    this.bindData();
  }

  initProcessList() {
    this.boards.processBoard = blessed.list({
      ...TABLE_BASE_OPTIONS,
      label: ' Processes ',
      parent: this.screen,
      padding: DEFAULT_PADDING,
      top: '0',
      left: '0',
      interactive: true,
      mouse: false,
      width: `${WIDTH_LEFT_PANEL}%`,
      height: '95%',
      style: {
        ...TABLE_BASE_OPTIONS.style,
        selected: {
          bold: true,
          bg: 'blue',
        },
      },
    }) as unknown as ListElementResolved;
    this.boards.processBoard.focus();
  }

  initLog() {
    this.boards.logBoard = blessed.list({
      ...TABLE_BASE_OPTIONS,
      label: ' Logs ',
      parent: this.screen,
      top: '0',
      left: `${WIDTH_LEFT_PANEL + 1}%`,
      width: `${100 - WIDTH_LEFT_PANEL - 1}%`,
      height: '95%',
      padding: DEFAULT_PADDING,
      mouse: true,
      style: {
        ...TABLE_BASE_OPTIONS.style,
        selected: {
          bold: true,
        },
      },
    }) as unknown as ListElementResolved;
    this.log = new Log();
  }

  initMessage() {
    this.boards.message = MessageBox(this.screen);
  }

  initLoader() {
    this.boards.loader = Loading(this.screen);
  }

  initBoxInfo() {
    this.boards.boxInfo = blessed.text({
      content:
        ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | Enter: start/stop process',
      parent: this.screen,
      left: '0%',
      top: '95%',
      width: '100%',
      height: '6%',
      valign: 'middle',
      tags: true,
    });
  }

  bindData() {
    processListObservable().subscribe((processList) => {
      if (!this.processList.length && processList.length) {
        const processId = processList[0].processId;
        this.log.initProcessId(processId);
        this.boards.logBoard.setLabel(` Logs (${processId}) `);
        this.boards.logBoard.setItems([]);
        this.screen.render();
      }

      this.processList = processList;
      const processListForRender = processList.map((item) => {
        return parseToString(
          [
            item.typeName,
            item.processName,
            item.statusName,
            'MEM ' + dealMemory(item.monit?.memory),
            'Cpu ' + (item.monit?.cpu || '0') + '%',
          ],
          [5, 15, 10, 10, 10],
        );
      });

      this.boards.processBoard.setItems(processListForRender);
      this.screen.render();
    });

    setTimerPromiseTask((): Promise<void> => {
      return new Promise((resolve) => {
        this._setLogItems();
        resolve();
      });
    }, 2000);
  }

  bindEvent() {
    this.screen.key(['C-c'], () => {
      this.screen.destroy();
      process.exit(0);
    });

    this.boards.processBoard.key(['enter'], () => {
      const selectedIndex: number = this.boards.processBoard.selected;
      const processItem = this.processList[selectedIndex];
      switchProcess(processItem, this.boards.message, this.boards.loader);
    });

    this.boards.processBoard.key(
      ['up', 'down'],
      debounce(() => {
        const selectedIndex: number = this.boards.processBoard.selected;
        const curProcessItem = this.processList[selectedIndex];
        this.log.initProcessId(curProcessItem.processId);
        this.boards.logBoard.setLabel(` Logs (${curProcessItem.processId}) `);
        this.boards.logBoard.setItems([]);
        this.screen.render();
      }, 300),
    );

    let i = 0;
    const boards = ['processBoard', 'logBoard'];
    this.screen.key(['left', 'right'], (ch: string, key: { name: string }) => {
      key.name === 'left' ? i-- : i++;
      if (i === 2) i = 0;
      if (i === -1) i = 1;
      const nameKey: string = boards[i];
      this.boards[nameKey].focus();
    });
  }

  _setLogItems() {
    if (!this.boards.logBoard.focused) {
      this.boards.logBoard.setItems(this.log.getLogs());
      this.boards.logBoard.select(this.log.getLogs().length - 1);
      this.boards.logBoard.setScrollPerc(100);
      this.screen.render();
    }
  }
}

export default () => {
  return new MonitorDashboard();
};
