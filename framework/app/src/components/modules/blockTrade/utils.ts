import {
  dealOrderInputItem,
  dealUnderweightType,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { VNode } from 'vue';
import { getBlockTradeOrderTrans } from './config';
import { createOrderPlaceVNode } from '../makeOrder/utils';
import {
  SideEnum,
  InstrumentTypeEnum,
  OffsetEnum,
  UnderweightEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

type OrderInputWithBlockMessage = KungfuApi.MakeOrderInput &
  KungfuApi.BlockMessage & {
    linkman: string;
    contact_way: string;
    underweight_type: UnderweightEnum;
  };

function dealStockOffset(
  makeOrderInput: OrderInputWithBlockMessage,
): OrderInputWithBlockMessage {
  if (makeOrderInput.instrument_type == InstrumentTypeEnum.stock) {
    if (makeOrderInput.side == SideEnum.Buy) {
      makeOrderInput.offset = OffsetEnum.Open;
    }
    if (makeOrderInput.side == SideEnum.Sell) {
      makeOrderInput.offset = OffsetEnum.Close;
    }
  }
  return makeOrderInput;
}

export const dealBlockMessageItem = (
  inputData: OrderInputWithBlockMessage,
): Record<string, KungfuApi.KfTradeValueCommonData> => {
  const orderInputResolved: Record<string, KungfuApi.KfTradeValueCommonData> =
    {};
  for (const key in inputData) {
    if (key === 'underweight_type') {
      orderInputResolved[key] = dealUnderweightType(inputData.underweight_type);
    }
  }
  return orderInputResolved;
};

export function dealOrderPlaceVNode(
  makeOrderInput: KungfuApi.MakeOrderInput & KungfuApi.BlockMessage,
  orderCount: number,
): VNode {
  const orderInputWithBlockMessage: OrderInputWithBlockMessage = {
    ...makeOrderInput,
    ...(makeOrderInput.value as {
      linkman: string;
      contact_way: string;
      underweight_type: UnderweightEnum;
    }),
  };

  const orderData: OrderInputWithBlockMessage = dealStockOffset(
    orderInputWithBlockMessage,
  );

  const currentOrderInputTrans = getBlockTradeOrderTrans();

  const blockMessageResolved: Record<string, KungfuApi.KfTradeValueCommonData> =
    dealBlockMessageItem(orderData);

  const orderInputResolved: Record<string, KungfuApi.KfTradeValueCommonData> =
    dealOrderInputItem(orderData);

  return createOrderPlaceVNode(
    { ...orderInputResolved, ...blockMessageResolved },
    currentOrderInputTrans,
    orderCount,
  );
}
