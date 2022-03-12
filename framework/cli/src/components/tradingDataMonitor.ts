import Loading from '../assets/components/Loading';
import MessageBox from '../assets/components/MessageBox';
import Dashboard from '../assets/components/Dashboard';
import buildOrderTable, { OrderTable } from '../assets/components/OrderTable';
import buildTradeTable, { TradeTable } from '../assets/components/TradeTable';
import buildPosTable, { PosTable } from '../assets/components/PosTable';
import { TABLE_BASE_OPTIONS, DEFAULT_PADDING } from '../assets/config';
import { ListElementResolved, ProcessListItem } from '../typings';
import blessed, { Widgets } from 'blessed';
import { specificProcessListObserver } from '../assets/actions/processList';
import { colorNum, dealMemory, parseToString } from '../assets/methods/utils';
import { switchProcess } from '../assets/actions/processList';
import {
  assetObservable,
  getAsset,
  getOrders,
  getPosition,
  getTrades,
  ordersObservable,
  posObservable,
  tradesObservable,
} from '../assets/actions/tradingData';
import { setTimerPromiseTask } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;
const WIDTH_PROCESS_PANEL = 30;

class TradingDataDashboard extends Dashboard {
  kfLocation: KungfuApi.KfConfig;
  boards: {
    processBoard: ListElementResolved | null;
    cancelBtn: Widgets.ButtonElement | null;
    message: Widgets.MessageElement | null;
    loader: Widgets.LoadingElement | null;
    boxInfo: Widgets.TextElement | null;
    orderTable: OrderTable | null;
    tradeTable: TradeTable | null;
    posTable: PosTable | null;
    assetTable: ListElementResolved | null;
  };

  processList: ProcessListItem[];

  constructor(kfLocation: KungfuApi.KfConfig) {
    super();
    this.screen.title =
      kfLocation.category === 'td' ? 'Td Dashboard' : 'Strategy Dashboard';
    this.kfLocation = kfLocation;

    this.boards = {
      processBoard: null,
      cancelBtn: null,
      message: null,
      loader: null,
      boxInfo: null,
      orderTable: null,
      tradeTable: null,
      posTable: null,
      assetTable: null,
    };

    this.processList = [];
    this.init();
  }

  init() {
    this.initProcessTable();
    this.initAssetTable();
    this.initPosTable();
    this.initOrderList();
    this.initTradeList();
    this.initBoxInfo();
    this.initMessage();
    this.initLoader();
    this.initCancelOrderBtn();
    this.screen.render();
    this.bindEvent();
    this.bindData();
  }

  initProcessTable() {
    this.boards.processBoard = blessed.list({
      ...TABLE_BASE_OPTIONS,
      label:
        this.kfLocation.category === 'td'
          ? ' Account Trading Engines '
          : ' Strategy Trading Engines ',
      parent: this.screen,
      padding: DEFAULT_PADDING,
      top: '0',
      left: '0',
      interactive: true,
      mouse: false,
      width: `${WIDTH_PROCESS_PANEL}%`,
      height: '40%',
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

  initAssetTable() {
    this.boards.assetTable = blessed.list({
      ...TABLE_BASE_OPTIONS,
      label: ' Assets ',
      parent: this.screen,
      padding: DEFAULT_PADDING,
      top: 0,
      left: `${WIDTH_PROCESS_PANEL}%+1`,
      interactive: true,
      mouse: true,
      width: `${WIDTH_LEFT_PANEL - WIDTH_PROCESS_PANEL}%-1`,
      height: '40%-3',
      style: {
        ...TABLE_BASE_OPTIONS.style,
        selected: {
          bold: true,
        },
      },
    }) as unknown as ListElementResolved;
    return false;
  }

  initCancelOrderBtn() {
    this.boards.cancelBtn = blessed.button({
      content: 'Cancel All Order',
      parent: this.screen,
      width: `${WIDTH_LEFT_PANEL - WIDTH_PROCESS_PANEL}%-1`,
      height: 3,
      left: `${WIDTH_PROCESS_PANEL}%+1`,
      top: '40%-3',
      align: 'center',
      valign: 'middle',
      interactive: true,
      mouse: true,
      border: 'line',
      style: {
        fg: 'white',
        bg: 'red',
        border: {
          fg: 'white',
        },
        focus: {
          border: {
            fg: 'red',
          },
        },
      },
    });
    this.boards.cancelBtn.setIndex(999);
  }

  initPosTable() {
    this.boards.posTable = buildPosTable();
    this.boards.posTable.build({
      label: ' Positions ',
      parent: this.screen,
      top: '0',
      left: `${WIDTH_LEFT_PANEL}%+1`,
      width: `${100 - WIDTH_LEFT_PANEL}%-1`,
      height: '40%',
    });
    return;
  }

  initOrderList() {
    this.boards.orderTable = buildOrderTable(this.kfLocation.category);
    this.boards.orderTable.build({
      label: ' Orders Records ',
      parent: this.screen,
      top: '40%',
      width: `${WIDTH_LEFT_PANEL}%`,
      height: '55%',
    });
  }

  initTradeList() {
    this.boards.tradeTable = buildTradeTable(this.kfLocation.category);
    this.boards.tradeTable.build({
      label: ' Trades Records ',
      parent: this.screen,
      top: '40%',
      left: `${WIDTH_LEFT_PANEL}%+1`,
      width: `${100 - WIDTH_LEFT_PANEL}%-1`,
      height: '55%',
    });
    return false;
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

  initMessage() {
    this.boards.message = MessageBox(this.screen);
  }

  initLoader() {
    this.boards.loader = Loading(this.screen);
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
  }

  bindData() {
    setTimerPromiseTask(
      () =>
        Promise.all([
          getOrders(this.kfLocation),
          getTrades(this.kfLocation),
          getPosition(this.kfLocation),
          getAsset(this.kfLocation),
        ]),
      3000,
    );

    specificProcessListObserver(this.kfLocation).subscribe((processList) => {
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

    ordersObservable(this.kfLocation).subscribe(
      (orders: KungfuApi.OrderResolved[]) => {
        this.boards.orderTable.setItems(orders);
      },
    );

    tradesObservable(this.kfLocation).subscribe(
      (trades: KungfuApi.TradeResolved[]) => {
        this.boards.tradeTable.setItems(trades);
      },
    );

    posObservable(this.kfLocation).subscribe(
      (positions: KungfuApi.PositionResolved[]) => {
        this.boards.posTable.setItems(positions);
      },
    );

    assetObservable(this.kfLocation).subscribe((asset: KungfuApi.Asset) => {
      this.boards.assetTable.setItems(this._freshAssets(asset));
    });
  }

  _freshAssets(assets: KungfuApi.Asset) {
    const assetsCloned: KungfuApi.Asset = JSON.parse(JSON.stringify(assets));

    if (this.kfLocation.category === 'td') {
      delete assetsCloned.client_id;
    } else if (this.kfLocation.category === 'strategy') {
      delete assetsCloned.account_id;
    }

    return Object.entries(assetsCloned).map((kvPair: string[]) => {
      let key = kvPair[0];
      let val = kvPair[1];
      const theFirst = key[0];
      key = theFirst.toUpperCase() + key.slice(1);
      if (['unrealized_pnl', 'realized_pnl'].includes(key)) {
        val = colorNum(+val);
      }
      return parseToString([key, val], [14, 14]);
    });
  }
}

export default (kfLocation: KungfuApi.KfConfig) =>
  new TradingDataDashboard(kfLocation);
