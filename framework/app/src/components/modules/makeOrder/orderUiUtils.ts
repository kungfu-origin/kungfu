import { dealOrderInputItem } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { h, VNode } from 'vue';
import { orderInputTrans } from './config';

export function dealOrderPlaceVNode(
  makeOrderInput: KungfuApi.MakeOrderInput,
  orderCount: number,
): VNode {
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
        h('span', { class: 'label' }, `${orderInputTrans[key]}`),
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
