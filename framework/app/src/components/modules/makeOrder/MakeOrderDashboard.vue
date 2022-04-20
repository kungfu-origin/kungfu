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
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { getConfigSettings, confirmModal } from './config';
import { message } from 'ant-design-vue';
import { makeOrderByOrderInput, hashInstrumentUKey } from '@kungfu-trader/kungfu-js-api/kungfu';
import { InstrumentTypeEnum, SideEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { getKfGlobalSettingsValue } from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealOrderInputItem,
  dealTradingData,
  getProcessIdByKfLocation,
  initFormStateByConfig,
  isTdStrategyCategory,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import OrderConfirmModal from './OrderConfirmModal.vue';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

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
const orderMessage = ref<number>(0)
const orderData = ref<KungfuApi.MakeOrderInput>();
const initialSideType: {
  value: number
} = {
  value: 0
}
const { getExtraCategoryData } = useExtraCategory();

const currentPosition = {
  position: '0',
}

const curPositionList = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return {};
  }

  const positions = isTdStrategyCategory(
    currentGlobalKfLocation.value.category,
  )
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
  return positions
})


onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$globalBus.subscribe((data: KfBusEvent) => {
      if (data.tag === 'makeOrder') {
        const { offset, side, volume, price, instrumentType, accountId } = (
          data as TriggerMakeOrder
        ).orderInput;
        initialSideType.value = +side;

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
    const { exchangeId, instrumentId, instrumentType } = instrumentResolved as KungfuApi.InstrumentResolved;
    const orderSymbol = `${instrumentId}_${exchangeId}_${instrumentType}`;
    dealPositionList(curPositionList.value, orderSymbol)
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

function dealPositionList(positionList, orderSymbol) {
  for (let key in positionList) {
    const itemOrderSymbol = `${positionList[key].instrument_id}_${positionList[key].exchange_id}_${positionList[key].instrument_type}`
    if (itemOrderSymbol === orderSymbol) {
      currentPosition.position = (positionList[key].volume as bigint).toString();
    }
  }
}

function placeOrder(
  orderInput: KungfuApi.MakeOrderInput,
  globallKfLocation,
  tdProcessId: string
) {
    makeOrderByOrderInput(
      window.watcher,
      orderInput,
      globallKfLocation,
      tdProcessId.toAccountId(),
    ).catch((err) => {
      message.error(err.message);
    });
}

function initInputData():Promise<KungfuApi.MakeOrderInput> {

  const instrument = formState.value.instrument.toString();
  const instrumnetResolved =
    transformSearchInstrumentResultToInstrument(instrument);

  if (!instrumnetResolved) {
    message.error('标的错误');
    return Promise.reject(new Error('标的错误'));
  }

  const { exchangeId, instrumentId, instrumentType } = instrumnetResolved;

  const {
    limit_price,
    volume,
    price_type,
    side,
    offset,
    hedge_flag,
  } = formState.value;

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
  return new Promise(resolve => {
    orderMessage.value = +makeOrderInput.volume
    orderData.value = makeOrderInput
    resolve(makeOrderInput);
  })

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
function handleApartOrder() {
  formRef.value
    .validate()
    .then(async () => {
      const makeOrderInput: KungfuApi.MakeOrderInput = await initInputData()
      const closeRange = +getKfGlobalSettingsValue()?.trade?.close || 0

      if (initialSideType.value !== makeOrderInput.side && makeOrderInput.volume.toString() === currentPosition.position && +makeOrderInput.volume > (closeRange * +currentPosition.position / 100)) {
        await confirmModal('提示', '是否全部平仓')
      }
      isShowConfirmModal.value = true;
      orderMessage.value = +makeOrderInput.volume
      orderData.value = makeOrderInput
  })
}

// 拆单弹窗确认回调
function handleApartedConfirm(amount: number): void {
  const remainder = orderMessage.value % +amount
  const eachAmount = Math.floor(orderMessage.value / +amount)
  const account_id = formState.value.account_id
  if (!currentGlobalKfLocation.value || !window.watcher) {
    message.error('当前 Location 错误');
    return;
  }

  const tdProcessId =
    currentGlobalKfLocation.value?.category === 'td'
      ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
      : `td_${account_id.toString()}`;

  if (processStatusData.value[tdProcessId] !== 'online') {
    message.error(`请先启动 ${tdProcessId} 交易进程`);
    return;
  }
  confirmOrderPlace(orderData.value as KungfuApi.MakeOrderInput, +remainder === 0 ? amount : amount + 1).then(() => {
    if (orderData.value !== undefined) {
      for (let i = 1; i <= +amount; i++) {
        if (i === +amount) {
          orderData.value.volume = eachAmount + remainder
        } else {
          orderData.value.volume = eachAmount
        }
        placeOrder(orderData.value, currentGlobalKfLocation.value, tdProcessId)
      }
    }
  })
}

function handleFatFinger(makeOrderInput: KungfuApi.MakeOrderInput): Promise<void> {
  return new Promise(resolve => {
    const instrument = formState.value.instrument.toString();
    const instrumnetResolved =
      transformSearchInstrumentResultToInstrument(instrument);
    if (!instrumnetResolved) {
      message.error('标的错误');
      return;
    }
    const fatFingerRange = +getKfGlobalSettingsValue()?.trade?.fatFinger || 0

    const { exchangeId, instrumentId } = instrumnetResolved;
  
    const {limit_price: price, side } = makeOrderInput
    const ukey = hashInstrumentUKey(instrumentId, exchangeId);
    const lastPrice = window.watcher.ledger.Quote[ukey].last_price
    const fatFingerBuyRate = fatFingerRange === 0 ? 100 : (100 + fatFingerRange) / 100
    const fatFingerSellRate = fatFingerRange === 0 ? 0 : (100 - fatFingerRange) / 100
    const warnningMessage = {
      message: ``,
    }
    
    if (SideEnum.Buy == side && price > lastPrice * fatFingerBuyRate) {
      warnningMessage.message = `买入价格超出警戒线，当前价格为${price}，警戒线为${(lastPrice * fatFingerBuyRate).toFixed(4)}, 当前乌龙指阈值为${fatFingerRange}%`
    }
    if (SideEnum.Sell == side && price < lastPrice * fatFingerSellRate) {
      warnningMessage.message = `卖出价格低于警戒线，当前价格为${price}，警戒线为${(lastPrice * fatFingerSellRate).toFixed(4)}, 当前乌龙指阈值为${fatFingerRange}%`
    }
    if (warnningMessage.message !== '') {
      confirmModal('警告', warnningMessage.message, '继续下单').then(() => {
        resolve()
      })
    } else {
      resolve()
    }
  })
}

function confirmOrderPlace(makeOrderInput: KungfuApi.MakeOrderInput, orderCount: number = 1): Promise<string> {
  
  return new Promise(resolve => {
    const { account_id } = formState.value;

    const orderInput: Record<string, KungfuApi.KfTradeValueCommonData> = dealOrderInputItem(makeOrderInput);
      
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
    
    const rootVNode = h('div', { class: 'root-node' }, [
      h('div', { class: 'trading-data-detail__warp' }, vnode),
      h('div', { class: 'ant-statistic apart-result-statistic order-number' }, [
        h('div', { class: 'ant-statistic-title', style: 'font-size: 16px' }, '下单次数'),
        h('div', { class: 'ant-statistic-content', style: 'font-size: 35px' }, `${orderCount}`),
      ])
    ])

    confirmModal('下单确认', h('div', { class: 'modal-node' }, rootVNode)).then(() => {
      if (!currentGlobalKfLocation.value || !window.watcher) {
        message.error('当前 Location 错误');
        return;
      }

      const tdProcessId =
        currentGlobalKfLocation.value?.category === 'td'
          ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
          : `td_${account_id.toString()}`;

      if (processStatusData.value[tdProcessId] !== 'online') {
        message.error(`请先启动 ${tdProcessId} 交易进程`);
        return;
      }

      resolve(tdProcessId)
    })
  })
}


function handleMakeOrder() {
  formRef.value.validate().then(async () => {
    const makeOrderInput: KungfuApi.MakeOrderInput = await initInputData()
    const closeRange = +getKfGlobalSettingsValue()?.trade?.close || 0
    if (initialSideType.value !== makeOrderInput.side && makeOrderInput.volume.toString() === currentPosition.position && +makeOrderInput.volume > closeRange) {
      await confirmModal('提示', '是否全部平仓').then(async () => {
        await handleFatFinger(makeOrderInput).then(async () => {
          await confirmOrderPlace(makeOrderInput).then((tdProcessId) => {
            placeOrder(makeOrderInput, currentGlobalKfLocation.value, tdProcessId)
          })
        })
      })
    } else {
      await handleFatFinger(makeOrderInput).then(async () => {
        await confirmOrderPlace(makeOrderInput).then((tdProcessId) => {
          placeOrder(makeOrderInput, currentGlobalKfLocation.value, tdProcessId)
        })
      })
    }
  }).catch((err: Error) => {
    console.error(err);
  })
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
        <div class="make-order-position ant-row ant-form-item ant-form-item-has-success" v-if="formState.instrument">
          <div class="position-label ant-col ant-col-5 ant-form-item-label">持有量:&nbsp</div>
          <div class="position-value ant-col ant-col-14 ant-form-item-control">{{currentPosition.position}}</div>
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
      :orderMessage="orderMessage"
      @confirm="handleApartedConfirm"
    >
    </OrderConfirmModal>
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
