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
import { parseToString } from '../assets/methods/utils';

const WIDTH_LEFT_PANEL = 35;

export class MonitorDashboard extends Dashboard {
  boards: {
    processBoard: ListElementResolved | null;
    message: Widgets.MessageElement | null;
    loader: Widgets.LoadingElement | null;
  };

  processList: ProcessListItem[];

  constructor() {
    super();
    this.screen.title = 'Processes Dashboard';

    this.boards = {
      processBoard: null,
      message: null,
      loader: null,
    };

    this.processList = [];
    this.init();
  }

  init() {
    this.initProcessList();
    this.initLog();
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
      width: WIDTH_LEFT_PANEL + '%',
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
    return false;
  }

  initMessage() {
    this.boards.message = MessageBox(this.screen);
  }

  initLoader() {
    this.boards.loader = Loading(this.screen);
  }

  bindData() {
    processListObservable().subscribe((processList) => {
      this.processList = processList;
      const processListForRender = processList.map((item) => {
        return parseToString(
          [
            item.typeName,
            item.processName,
            item.statusName,
            'Cpu ' + (item.monit?.cpu || '--'),
            'MEM ' + (item.monit?.memory || '--'),
          ],
          [5, 15, 10, 10, 10],
        );
      });

      this.boards.processBoard.setItems(processListForRender);
      this.screen.render();
    });
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

    // t.boards.processList.key(
    //   ['up', 'down'],
    //   debounce((ch: string, key: any) => {
    //     const selectedIndex: number = t.boards.processList.selected;
    //     const curProcessItem = t.globalData.processList[selectedIndex];
    //     t._getLogs(curProcessItem);
    //   }, 1000),
    // );
  }
}

export default () => {
  return new MonitorDashboard();
};
