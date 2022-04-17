import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  HedgeFlagEnum,
  InstrumentTypeEnum,
  KfCategoryTypes,
  OffsetEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { dealOrderInputItem } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Modal } from 'ant-design-vue';
import { VueNode } from 'ant-design-vue/lib/_util/type';
import { h } from 'vue';

export const getConfigSettings = (
  category?: KfCategoryTypes,
  instrumentTypeEnum?: InstrumentTypeEnum,
  priceType?: PriceTypeEnum,
): KungfuApi.KfConfigItem[] => {
  const shotable = instrumentTypeEnum
    ? ShotableInstrumentTypes.includes(
        instrumentTypeEnum || InstrumentTypeEnum.unknown,
      )
    : false;

  const defaultSettings: KungfuApi.KfConfigItem[] = [
    category === 'td'
      ? null
      : {
          key: 'account_id',
          name: '账户',
          type: 'td',
          required: true,
        },
    {
      key: 'instrument',
      name: '标的',
      type: 'instrument',
      required: true,
    },
    {
      key: 'side',
      name: '买卖',
      type: 'side',
      default: SideEnum.Buy,
      required: true,
    },
    ...(shotable
      ? [
          {
            key: 'offset',
            name: '开平',
            type: 'offset',
            default: OffsetEnum.Open,
            required: true,
          },
          {
            key: 'hedge_flag',
            name: '套保',
            type: 'hedgeFlag',
            default: HedgeFlagEnum.Speculation,
            required: true,
          },
        ]
      : []),
    {
      key: 'price_type',
      name: '方式',
      type: 'priceType',
      default: PriceTypeEnum.Limit,
      required: true,
    },
    {
      key: 'limit_price',
      name: priceType === PriceTypeEnum.Limit ? '价格' : '保护价格',
      type: 'float',
      required: priceType === PriceTypeEnum.Limit ? true : false,
    },
    {
      key: 'volume',
      name: '下单量',
      type: 'int',
      required: true,
    },
  ].filter((item) => !!item) as KungfuApi.KfConfigItem[];

  return defaultSettings;
};

export const confirmModal = (
  title: string,
  content: VueNode | (() => VueNode),
  okText = '确 定',
  cancelText = '取 消',
): Promise<void> => {
  return new Promise((resolve) => {
    Modal.confirm({
      title: title,
      content: content,
      okText: okText,
      cancelText: cancelText,
      onOk: () => {
        resolve();
      },
    });
  });
};

const orderconfig: Record<string, string> = {
  account_id: '账户',
  instrument_id: '标的ID',
  instrument_type: '标的类型',
  side: '买卖',
  offset: '开平',
  hedge_flag: '套保',
  price_type: '方式',
  volume: '下单量',
  exchange_id: '交易所ID',
  limit_price: '限额',
};

export const vNodeConfirmModal = (
  makeOrderInput: KungfuApi.MakeOrderInput,
): Promise<void> => {
  return new Promise((resolve) => {
    const orderInput: Record<string, KungfuApi.KfTradeValueCommonData> =
      dealOrderInputItem(makeOrderInput);
    const vnode = Object.keys(orderInput)
      .filter((key) => {
        if (orderInput[key].name.toString() === '[object Object]') {
          return false;
        }
        return orderInput[key].name !== '';
      })
      .map((key) =>
        h('div', { class: 'trading-data-detail-row' }, [
          h('span', { class: 'label' }, `${orderconfig[key]}`),
          h(
            'span',
            {
              class: `value ${orderInput[key].color}`,
              style: { color: `${orderInput[key].color}` },
            },
            `${orderInput[key].name}`,
          ),
        ]),
      );
    console.log(h('div', { class: 'trading-data-detail__warp', vnode }));
    Modal.confirm({
      title: '下单确认',
      content: h('div', { class: 'trading-data-detail__warp', vnode }),
      okText: '确 定',
      cancelText: '取 消',
      onOk: () => {
        resolve();
      },
    });
  });
};
