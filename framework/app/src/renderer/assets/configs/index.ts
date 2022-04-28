import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

export const defaultBoardsMap: KfLayout.BoardsMap = {
  '0': { paId: -1, direction: 'h', children: [1, 2] },
  '1': { paId: 0, direction: 'v', children: [4, 5], width: '64.620%' },
  '2': { paId: 0, direction: 'v', children: [8, 9, 10], width: '35.380%' },
  '4': {
    paId: 1,
    direction: 'h',
    contents: [t('Td.name')],
    current: t('Td.name'),
    height: '23.294%',
    width: 0,
  },
  '5': {
    paId: 1,
    direction: 'h',
    children: [14, 13],
    width: 0,
    height: '76.706%',
  },
  '8': {
    paId: 2,
    direction: 'h',
    contents: [t('Md.name')],
    current: t('Md.name'),
    height: '17.577%',
  },
  '9': {
    paId: 2,
    direction: 'h',
    contents: [t('Pos.name')],
    current: t('Pos.name'),
    height: '28.157%',
  },
  '10': { paId: 2, direction: 'h', children: [11, 12], height: '54.266%' },
  '11': {
    paId: 10,
    direction: 'v',
    contents: [t('OrderBook.name')],
    current: t('OrderBook.name'),
    width: '43.279%',
  },
  '12': {
    paId: 10,
    direction: 'v',
    contents: [t('MakeOrderDashboard.name'), t('FutureArbitrage.name')],
    current: t('MakeOrderDashboard.name'),
    width: '56.721%',
  },
  '13': {
    paId: 5,
    direction: 'v',
    contents: [t('MarketData.name')],
    current: t('MarketData.name'),
    width: '23.045%',
  },
  '14': { paId: 5, direction: 'v', width: '76.955%', children: [16, 15] },
  '15': {
    paId: 14,
    direction: 'h',
    contents: [t('Order.name'), t('Trade.name')],
    current: t('Order.name'),
    width: 0,
    height: 0,
  },
  '16': {
    paId: 14,
    direction: 'h',
    contents: [t('Strategy.name'), t('TradingTask.name'), t('PosGlobal.name')],
    current: t('Strategy.name'),
  },
};

export const getIndexBoardsMap = (): KfLayout.BoardsMap | null => {
  const data = localStorage.getItem('indexBoardsMap');
  if (!data) {
    return null;
  }
  const storedBoardsMap = JSON.parse(data) as KfLayout.BoardsMap;
  if (!Object.keys(storedBoardsMap).length) {
    return null;
  }

  return storedBoardsMap;
};

export const coreForScheduleTasksOptions: Array<{
  label: string;
  value: string;
  category: KfCategoryTypes;
}> = [
  {
    label: '主控进程',
    value: 'core',
    category: 'system',
  },
];

export const modeForScheduleTasksOptions: Array<{
  label: string;
  value: KungfuApi.ScheduleTaskMode;
}> = [
  {
    label: '启动',
    value: 'start',
  },
  {
    label: '停止',
    value: 'stop',
  },
];

export const modeForCoreScheduleTasksOptions: Array<{
  label: string;
  value: KungfuApi.ScheduleTaskMode;
}> = [
  {
    label: '重启',
    value: 'restart',
  },
  {
    label: '启动',
    value: 'start',
  },
  {
    label: '停止',
    value: 'stop',
  },
];
