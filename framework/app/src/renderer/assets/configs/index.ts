import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const defaultBoardsMap: KfLayout.BoardsMap = {
  '0': { paId: -1, direction: 'h', children: [1, 2] },
  '1': { paId: 0, direction: 'v', children: [4, 5], width: '64.620%' },
  '2': { paId: 0, direction: 'v', children: [8, 9, 10], width: '35.380%' },
  '4': {
    paId: 1,
    direction: 'h',
    contents: [t('Td')],
    current: t('Td'),
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
    contents: [t('Md')],
    current: t('Md'),
    height: '17.577%',
  },
  '9': {
    paId: 2,
    direction: 'h',
    contents: [t('Pos')],
    current: t('Pos'),
    height: '28.157%',
  },
  '10': { paId: 2, direction: 'h', children: [11, 12], height: '54.266%' },
  '11': {
    paId: 10,
    direction: 'v',
    contents: [t('OrderBook')],
    current: t('OrderBook'),
    width: '43.279%',
  },
  '12': {
    paId: 10,
    direction: 'v',
    contents: [t('MakeOrderDashboard'), t('FutureArbitrage')],
    current: t('MakeOrderDashboard'),
    width: '56.721%',
  },
  '13': {
    paId: 5,
    direction: 'v',
    contents: [t('MarketData')],
    current: t('MarketData'),
    width: '23.045%',
  },
  '14': { paId: 5, direction: 'v', width: '76.955%', children: [16, 15] },
  '15': {
    paId: 14,
    direction: 'h',
    contents: [t('Order'), t('Trade')],
    current: t('Order'),
    width: 0,
    height: 0,
  },
  '16': {
    paId: 14,
    direction: 'h',
    contents: [t('Strategy'), t('TradingTask'), t('PosGlobal')],
    current: t('Strategy'),
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
    label: t('globalSettingConfig.master'),
    value: 'core',
    category: 'system',
  },
];

export const modeForScheduleTasksOptions: Array<{
  label: string;
  value: KungfuApi.ScheduleTaskMode;
}> = [
  {
    label: t('globalSettingConfig.start'),
    value: 'start',
  },
  {
    label: t('globalSettingConfig.stop'),
    value: 'stop',
  },
];

export const modeForCoreScheduleTasksOptions: Array<{
  label: string;
  value: KungfuApi.ScheduleTaskMode;
}> = [
  {
    label: t('globalSettingConfig.restart'),
    value: 'restart',
  },
  {
    label: t('globalSettingConfig.start'),
    value: 'start',
  },
  {
    label: t('globalSettingConfig.stop'),
    value: 'stop',
  },
];
