<script setup lang="ts">
import {
  computed,
  getCurrentInstance,
  h,
  nextTick,
  onBeforeUnmount,
  onMounted,
  ref,
  watch,
} from 'vue';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfConfigSettingsForm from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfConfigSettingsForm.vue';
import {
  buildInstrumentSelectOptionValue,
  useTriggerMakeOrder,
  useDashboardBodySize,
  confirmModal,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { getConfigSettings } from './config';
import { dealOrderPlaceVNode } from './orderUiUtils'
import { message } from 'ant-design-vue';
import {
  makeOrderByOrderInput,
  hashInstrumentUKey,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  InstrumentTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { getKfGlobalSettingsValue } from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealTradingData,
  getProcessIdByKfLocation,
  initFormStateByConfig,
  isTdStrategyCategory,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import OrderConfirmModal from './OrderConfirmModal.vue';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();
const formState = ref(
  initFormStateByConfig(getConfigSettings('td', InstrumentTypeEnum.future), {}),
);

const formRef = ref();
const { subscribeAllInstrumentByAppStates } = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap } = useExtConfigsRelated();
const { triggerOrderBook } = useTriggerMakeOrder();

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const makeOrderInstrumentType = ref<InstrumentTypeEnum>(
  InstrumentTypeEnum.unknown,
);

const configSettings = computed(() => {
  if (!currentGlobalKfLocation.value) {
    return getConfigSettings();
  }

  const { category } = currentGlobalKfLocation.value;
  return getConfigSettings(
    category,
    makeOrderInstrumentType.value,
    +formState.value.price_type,
  );
});

const isShowConfirmModal = ref<boolean>(false);
const curOrderVolume = ref<number>(0);
const orderData = ref<KungfuApi.MakeOrderInput>();

const { getExtraCategoryData } = useExtraCategory();

const currentPosition: {
  position: KungfuApi.Position | null;
} = {
  position: null,
};

const curPositionList = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return [];
  }

  const positions = isTdStrategyCategory(currentGlobalKfLocation.value.category)
    ? ((dealTradingData(
        window.watcher,
        window.watcher.ledger,
        'Position',
        currentGlobalKfLocation.value,
      ) || []) as KungfuApi.Position[])
    : (getExtraCategoryData(
        window.watcher.ledger.Position,
        currentGlobalKfLocation.value,
        'position',
      ) as KungfuApi.Position[]);
  return positions;
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$globalBus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'makeOrder') {
        const { offset, side, volume, price, instrumentType, accountId } = (
          data as TriggerMakeOrder
        ).orderInput;

        const instrumentValue = buildInstrumentSelectOptionValue(
          (data as TriggerMakeOrder).orderInput,
        );

        formState.value.instrument = instrumentValue;
        formState.value.offset = +offset;
        formState.value.side = +side;
        formState.value.volume = +Number(volume).toFixed(0);
        formState.value.limit_price = +Number(price).toFixed(4);
        formState.value.instrument_type = +instrumentType;

        if (accountId) {
          formState.value.account_id = accountId;
        }
      }

      if (data.tag === 'orderBookUpdate') {
        const { side, price, volume, instrumentType } = (
          data as TriggerOrderBookUpdate
        ).orderInput;

        const instrumentValue = buildInstrumentSelectOptionValue(
          (data as TriggerOrderBookUpdate).orderInput,
        );

        if (!formState.value.instrument) {
          formState.value.instrument = instrumentValue;
          formState.value.instrument_type = +instrumentType;
        }
        if (+price !== 0) {
          formState.value.limit_price = +Number(price).toFixed(4);
        }
        if (BigInt(volume) !== BigInt(0)) {
          formState.value.volume = +Number(volume).toFixed(0);
        }

        formState.value.side = +side;
      }
    });

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(
  () => formState.value.instrument,
  (newVal) => {
    const instrumentResolved = transformSearchInstrumentResultToInstrument(
      newVal.toString(),
    );

    if (!instrumentResolved) {
      return;
    }

    dealPositionList(curPositionList.value, instrumentResolved);

    subscribeAllInstrumentByAppStates(
      processStatusData.value,
      appStates.value,
      mdExtTypeMap.value,
      [instrumentResolved],
    );

    triggerOrderBook(instrumentResolved);

    makeOrderInstrumentType.value = instrumentResolved.instrumentType;
  },
);

function dealPositionList(positionList: KungfuApi.Position[], instrumentResolved: KungfuApi.InstrumentResolved) {
  const { exchangeId, instrumentId, instrumentType } =
    instrumentResolved

  const targetPosition: KungfuApi.Position = positionList.filter(position => position.exchange_id === exchangeId && position.instrument_id === instrumentId && position.instrument_type === instrumentType)[0];

  currentPosition.position = targetPosition;
}

function placeOrder(
  orderInput: KungfuApi.MakeOrderInput,
  globalKfLocation: KungfuApi.KfLocation,
  tdProcessId: string,
): Promise<void> {
  return makeOrderByOrderInput(
    window.watcher,
    orderInput,
    globalKfLocation,
    tdProcessId.toAccountId(),
  ).then();
}

function initOrderInputData(): Promise<KungfuApi.MakeOrderInput> {
  const instrument = formState.value.instrument.toString();
  const instrumnetResolved =
    transformSearchInstrumentResultToInstrument(instrument);

  if (!instrumnetResolved) {
    message.error('标的错误');
    return Promise.reject(new Error('标的错误'));
  }

  const { exchangeId, instrumentId, instrumentType } = instrumnetResolved;

  const { limit_price, volume, price_type, side, offset, hedge_flag } =
    formState.value;

  const makeOrderInput: KungfuApi.MakeOrderInput = {
    instrument_id: instrumentId,
    instrument_type: +instrumentType,
    exchange_id: exchangeId,
    limit_price: +limit_price,
    volume: +volume,
    price_type: +price_type,
    side: +side,
    offset: +(offset !== undefined ? offset : +side === 0 ? 0 : 1),
    hedge_flag: +(hedge_flag || 0),
    parent_id: BigInt(0),
  };

  return Promise.resolve(makeOrderInput);
}

function dealGlobalData(makeOrderInput: KungfuApi.MakeOrderInput) {
  orderData.value = makeOrderInput;
  curOrderVolume.value = +makeOrderInput.volume;
}

function handleResetMakeOrderForm() {
  const initFormState = initFormStateByConfig(configSettings.value, {});

  Object.keys(initFormState).forEach((key) => {
    formState.value[key] = initFormState[key];
  });

  nextTick().then(() => {
    formRef.value.clearValidate();
  });
}

// 拆单
async function handleApartOrder() {
  try {
    await formRef.value.validate();

    const makeOrderInput: KungfuApi.MakeOrderInput = await initOrderInputData();
    const closeRange = +getKfGlobalSettingsValue()?.trade?.close || 0;

    if (
      makeOrderInput.volume === Number(currentPosition.position?.volume) &&
      +makeOrderInput.volume > (closeRange * Number(currentPosition.position?.volume)) / 100
    ) {
      await confirmModal('提示', '是否全部平仓');
    }

    isShowConfirmModal.value = true;

    dealGlobalData(makeOrderInput);

  } catch (error) {
    message.error(error.message);
  }
}

// 拆单弹窗确认回调
async function handleApartedConfirm({volume, count}: {volume: number, count: number}): Promise<void> {
  
  const remainder: number = curOrderVolume.value % +volume; // 剩余数量
  
  const realCount: number = remainder === 0 ? count : count + 1;
  
  const apartVolumeList: number[] = new Array(count).fill(+volume);
  if (remainder !== 0) {
    apartVolumeList.push(remainder);
  }
  
  const account_id = formState.value.account_id;

  if (!currentGlobalKfLocation.value || !window.watcher) {
    message.error('当前 Location 错误');
    return;
  }

  const tdProcessId =
    currentGlobalKfLocation.value?.category === 'td'
      ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
      : `td_${account_id.toString()}`;

  if (!orderData.value || !currentGlobalKfLocation.value) {
    return;
  }

  await confirmOrderPlace(orderData.value, realCount)
  Promise.all(apartVolumeList.map(volume => {
    (orderData.value as KungfuApi.MakeOrderInput).volume = volume;
    return placeOrder((orderData.value as KungfuApi.MakeOrderInput), currentGlobalKfLocation.value as KungfuApi.KfLocation, tdProcessId);
  }))
}

function confirmFatFingerModal(
  makeOrderInput: KungfuApi.MakeOrderInput,
): Promise<void> {
  const warnningMessage = dealFatFingerMessage(makeOrderInput);

  if (warnningMessage !== '') {
    return confirmModal('警告', warnningMessage, '继续下单');
  } else {
    return Promise.resolve();
  }
}

function dealFatFingerMessage(
  makeOrderInput: KungfuApi.MakeOrderInput,
): string {
  const instrument = formState.value.instrument.toString();
  const instrumnetResolved =
    transformSearchInstrumentResultToInstrument(instrument);

  if (!instrumnetResolved) {
    message.error('标的错误');
    return '';
  }

  const fatFingerRange = +getKfGlobalSettingsValue()?.trade?.fatFinger || 0;

  const { exchangeId, instrumentId } = instrumnetResolved;

  const { limit_price: price, side } = makeOrderInput;
  const ukey = hashInstrumentUKey(instrumentId, exchangeId);
  const lastPrice = window.watcher.ledger.Quote[ukey].last_price;

  const fatFingerBuyRate =
    fatFingerRange === 0 ? 100 : (100 + fatFingerRange) / 100;
  const fatFingerSellRate =
    fatFingerRange === 0 ? 0 : (100 - fatFingerRange) / 100;

  if (SideEnum.Buy == side && price > lastPrice * fatFingerBuyRate) {
    return `买入价格超出警戒线，当前价格为${price}，警戒线为${(
      lastPrice * fatFingerBuyRate
    ).toFixed(4)}, 当前乌龙指阈值为${fatFingerRange}%`;
  }

  if (SideEnum.Sell == side && price < lastPrice * fatFingerSellRate) {
    return `卖出价格低于警戒线，当前价格为${price}，警戒线为${(
      lastPrice * fatFingerSellRate
    ).toFixed(4)}, 当前乌龙指阈值为${fatFingerRange}%`;
  }

  return '';
}

async function confirmOrderPlace(
  makeOrderInput: KungfuApi.MakeOrderInput,
  orderCount: number = 1,
): Promise<string> {
    const { account_id } = formState.value;

    await confirmModal(
      '下单确认',
      h(
        'div',
        { class: 'modal-node' },
        dealOrderPlaceVNode(makeOrderInput, orderCount),
      ),
    )
    if (!currentGlobalKfLocation.value || !window.watcher) {
      message.error('当前 Location 错误');
      return Promise.reject('当前 Location 错误');
    }

    const tdProcessId =
      currentGlobalKfLocation.value?.category === 'td'
        ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
        : `td_${account_id.toString()}`;

    if (processStatusData.value[tdProcessId] !== 'online') {
      message.error(`请先启动 ${tdProcessId} 交易进程`);
      return Promise.reject('请先启动交易进程');
    }

    return Promise.resolve(tdProcessId);
}

async function handleMakeOrder() {
  try {
    await formRef.value.validate();

    const makeOrderInput: KungfuApi.MakeOrderInput = await initOrderInputData();

    const closeRange = +getKfGlobalSettingsValue()?.trade?.close || 0;

    if (
      makeOrderInput.volume === Number(currentPosition.position?.volume) &&
      +makeOrderInput.volume > (closeRange * Number(currentPosition.position?.volume)) / 100
    ) {
      await confirmModal('提示', '是否全部平仓');
    }

    await confirmFatFingerModal(makeOrderInput);

    const tdProcessId = await confirmOrderPlace(makeOrderInput);

    if (currentGlobalKfLocation.value) {
      await placeOrder(
        makeOrderInput,
        currentGlobalKfLocation.value,
        tdProcessId,
      );
    }
  } catch (e) {
    message.error(e.message);
  }
}
</script>

<template>
  <div class="kf-make-order-dashboard__warp">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-button size="small" @click="handleResetMakeOrderForm">
            重置
          </a-button>
        </KfDashboardItem>
      </template>
      <div class="make-order__warp">
        <div class="make-order-form__warp">
          <KfConfigSettingsForm
            ref="formRef"
            v-model:formState="formState"
            :configSettings="configSettings"
            changeType="add"
            :label-col="5"
            :wrapper-col="14"
          ></KfConfigSettingsForm>
          <div
            class="make-order-position ant-row ant-form-item ant-form-item-has-success"
            v-if="formState.instrument"
          >
            <div class="position-label ant-col ant-col-5 ant-form-item-label">
              持有量:&nbsp
            </div>
            <div
              class="position-value ant-col ant-col-14 ant-form-item-control"
            >
              {{ currentPosition.position?.volume }}
            </div>
          </div>
        </div>
        <div class="make-order-btns">
          <a-button class="make-order" size="small" @click="handleMakeOrder">
            下单
          </a-button>
          <a-button size="small" @click="handleApartOrder">拆单</a-button>
        </div>
      </div>
    </KfDashboard>
    <OrderConfirmModal
      v-if="isShowConfirmModal"
      v-model:visible="isShowConfirmModal"
      :curOrderVolume="curOrderVolume"
      @confirm="handleApartedConfirm"
    ></OrderConfirmModal>
  </div>
</template>
<style lang="less">
.kf-make-order-dashboard__warp {
  width: 100%;
  height: 100%;

  .make-order__warp {
    display: flex;
    justify-content: space-between;
    height: 100%;

    .make-order-form__warp {
      flex: 1;
      height: 100%;
      padding-top: 16px;
      overflow-y: overlay;

      .ant-form-item {
        margin-bottom: 16px;

        .ant-form-item-explain,
        .ant-form-item-extra {
          min-height: unset;
        }
      }
    }
    .make-order-position {
      display: flex;
    }

    .make-order-btns {
      width: 40px;
      height: 100%;
      .ant-btn {
        height: 26%;
        text-align: center;
        word-break: break-word;
        word-wrap: unset;
        white-space: normal;
      }
      .make-order {
        margin-bottom: 8px;
        height: 72%;
      }
    }
  }
  .green {
    color: @green-base !important;
  }
  .red {
    color: @red-base !important;
  }
}
.modal-node {
  .root-node {
    display: flex;
    flex-wrap: nowrap;
    .green {
      color: @green-base !important;
    }
    .red {
      color: @red-base !important;
    }
    .order-number {
      flex: 1;
      margin-top: 10%;
      text-align: center;
    }
  }
}
</style>
