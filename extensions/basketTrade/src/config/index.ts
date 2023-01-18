import { BasketOrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';

export const BASKET_CATEGORYS = {
  SETTING: 'basketSetting',
  INSTRUMENT: 'basketInstrument',
  ORDER: 'basketOrder',
};

export const UnfinishedBasketOrderStatus = [
  BasketOrderStatusEnum.PartialFilledActive,
  BasketOrderStatusEnum.Pending,
];

export const defaultBasketTradeBoardsMap: KfLayout.BoardsMap = {
  '0': {
    paId: -1,
    direction: 'v',
    children: [1, 2],
  },
  '1': { paId: 0, direction: 'h', children: [3, 4], height: '50%' },
  '2': { paId: 0, direction: 'h', children: [5, 6], height: '50%' },
  '3': {
    paId: 1,
    direction: 'v',
    children: [7, 8],
    width: '60%',
    height: '100%',
  },
  '4': {
    paId: 1,
    direction: 'v',
    contents: ['Pos'],
    current: 'Pos',
    width: '40%',
    height: '100%',
  },
  '5': {
    paId: 2,
    direction: 'v',
    contents: ['篮子委托记录'],
    current: '篮子委托记录',
    width: '40%',
    height: '100%',
  },
  '6': {
    paId: 2,
    direction: 'v',
    contents: ['委托记录'],
    current: '委托记录',
    width: '60%',
    height: '100%',
  },
  '7': {
    paId: 3,
    direction: 'h',
    contents: ['Td'],
    current: 'Td',
    height: '48%',
    width: '100%',
  },
  '8': {
    paId: 3,
    direction: 'h',
    children: [9, 10],
    height: '52%',
    width: '100%',
  },
  '9': {
    paId: 8,
    direction: 'v',
    contents: ['篮子设置'],
    current: '篮子设置',
    width: '42%',
    height: '100%',
  },
  '10': {
    paId: 8,
    direction: 'v',
    contents: ['篮子详情'],
    current: '篮子详情',
    width: '58%',
    height: '100%',
  },
};
