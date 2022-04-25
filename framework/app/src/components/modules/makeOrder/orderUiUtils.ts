import {
  InstrumentTypeEnum,
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { dealOrderInputItem } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { h, VNode } from 'vue';
import { orderInputTrans } from './config';

export function dealOrderPlaceVNode(
  makeOrderInput: KungfuApi.MakeOrderInput,
  orderCount: number,
): VNode {
  if (makeOrderInput.instrument_type == InstrumentTypeEnum.stock) {
    if (makeOrderInput.side == SideEnum.Buy) {
      makeOrderInput.offset = OffsetEnum.Open;
    }
    if (makeOrderInput.side == SideEnum.Sell) {
      makeOrderInput.offset = OffsetEnum.Close;
    }
  }

  const orderInputResolved: Record<string, KungfuApi.KfTradeValueCommonData> =
    dealOrderInputItem(makeOrderInput);

  const vnode = Object.keys(orderInputResolved)
    .filter((key) => {
      if (orderInputResolved[key].name.toString() === '[object Object]') {
        return false;
      }
      return orderInputResolved[key].name !== '';
    })
    .map((key) =>
      h('div', { class: 'trading-data-detail-row' }, [
        h('span', { class: 'label' }, `${orderInputTrans[key]}`),
        h(
          'span',
          {
            class: `value ${orderInputResolved[key].color}`,
            style: { color: `${orderInputResolved[key].color}` },
          },
          `${orderInputResolved[key].name}`,
        ),
      ]),
    );

  const rootVNode = h('div', { class: 'root-node' }, [
    h('div', { class: 'trading-data-detail__warp' }, vnode),
    h('div', { class: 'ant-statistic apart-result-statistic order-number' }, [
      h(
        'div',
        { class: 'ant-statistic-title', style: 'font-size: 16px' },
        '下单次数',
      ),
      h(
        'div',
        { class: 'ant-statistic-content', style: 'font-size: 35px' },
        `${orderCount}`,
      ),
    ]),
  ]);

  return rootVNode;
}
