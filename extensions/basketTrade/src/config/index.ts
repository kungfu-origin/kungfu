export const BASKET_CATEGORYS = {
  SETTING: 'basketSetting',
  ORDER: 'basketOrder',
};

export const defaultBasketTradeBoardsMap: KfLayout.BoardsMap = {
  '0': {
    paId: -1,
    direction: 'v',
    children: [1, 2],
  },
  '1': { paId: 0, direction: 'h', children: [3, 4, 5], height: '40%' },
  '2': { paId: 0, direction: 'h', children: [6], height: '60%' },
  '3': {
    paId: 1,
    direction: 'v',
    contents: ['篮子设置'],
    current: '篮子设置',
    width: '33.4%',
    height: '100%',
  },
  '4': {
    paId: 1,
    direction: 'v',
    contents: ['篮子详情'],
    current: '篮子详情',
    width: '33.3%',
    height: '100%',
  },
  '5': {
    paId: 1,
    direction: 'v',
    contents: ['篮子委托记录'],
    current: '篮子委托记录',
    width: '33.3%',
    height: '100%',
  },
  '6': {
    paId: 2,
    direction: 'v',
    contents: ['委托记录'],
    current: '委托记录',
    width: '100%',
    height: '100%',
  },
  // '7': {
  //   paId: 2,
  //   direction: 'v',
  //   contents: ['成交记录'],
  //   current: '成交记录',
  //   width: '40%',
  //   height: '100%',
  // },
};
