<script setup lang="ts">
import {
  computed,
  getCurrentInstance,
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
  messagePrompt,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { getConfigSettings, LABEL_COL, WRAPPER_COL } from './config';
import {
  dealOrderPlaceVNode,
  dealStockOffset,
  transformOrderInputToExtConfigForm,
} from './utils';
import {
  makeOrderByOrderInput,
  hashInstrumentUKey,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  InstrumentTypeEnum,
  OffsetEnum,
  OrderInputKeyEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
  useTradeLimit,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  getExtConfigList,
  getIdByKfLocation,
  getProcessIdByKfLocation,
  initFormStateByConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import OrderConfirmModal from './OrderConfirmModal.vue';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { useTradingTask } from '../tradingTask/utils';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { storeToRefs } from 'pinia';
import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { useMakeOrderInfo } from '../../../renderer/assets/methods/actionsUtils';

const { t } = VueI18n.global;
const { error } = messagePrompt();

const app = getCurrentInstance();
const { instrumentKeyAccountsMap, uiExtConfigs } = storeToRefs(
  useGlobalStore(),
);

const { globalSetting } = storeToRefs(useGlobalStore());

const { handleBodySizeChange } = useDashboardBodySize();
const formState = ref(
  initFormStateByConfig(getConfigSettings('td', InstrumentTypeEnum.future), {}),
);

const formRef = ref();
const { subscribeAllInstrumentByAppStates } = useInstruments();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { mdExtTypeMap, extConfigs } = useExtConfigsRelated();
const { triggerOrderBook } = useTriggerMakeOrder();
const {
  showAmountOrPosition,
  instrumentResolved,
  currentPosition,
  currentResidueMoney,
  currentResiduePosVolume,
  currentPrice,
  currentTradeAmount,
  currentAvailMoney,
  currentAvailPosVolume,
  isAccountOrInstrumentConfirmed,
} = useMakeOrderInfo(app, formState);

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { getValidatorByOrderInputKey } = useTradeLimit();

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

const rules = computed(() => {
  const { instrument } = formState.value;
  return {
    volume: {
      validator: getValidatorByOrderInputKey(
        OrderInputKeyEnum.VOLUME,
        instrument,
      ),
      trigger: 'change',
    },
    limit_price: {
      validator: getValidatorByOrderInputKey(
        OrderInputKeyEnum.PRICE,
        instrument,
      ),
      trigger: 'change',
    },
  };
});

const isShowConfirmModal = ref<boolean>(false);
const curOrderVolume = ref<number>(0);
const curOrderType = ref<InstrumentTypeEnum>(InstrumentTypeEnum.unknown);
const currentPercent = ref<number>(0);
const percentList = [10, 20, 50, 80, 100];

const makeOrderData = computed(() => {
  if (!instrumentResolved.value) {
    return null;
  }

  const { exchangeId, instrumentId, instrumentType } = instrumentResolved.value;

  const { limit_price, volume, price_type, side, offset, hedge_flag, is_swap } =
    formState.value;

  const makeOrderInput: KungfuApi.MakeOrderInput = {
    instrument_id: instrumentId,
    instrument_type: +instrumentType,
    exchange_id: exchangeId,
    limit_price: +limit_price,
    volume: +volume,
    price_type: +price_type,
    side: +side,
    offset: getResolvedOffset(offset, side, instrumentType),
    hedge_flag: +(hedge_flag || 0),
    is_swap: !!is_swap,
  };
  return makeOrderInput;
});

const availTradingTaskExtensionList = computed(() => {
  return getExtConfigList(extConfigs.value, 'strategy').filter((item) => {
    const curExtConfig = uiExtConfigs.value[item.key];
    return (
      curExtConfig?.position === 'make_order' &&
      curExtConfig?.name !== 'ExcelTask'
    );
  });
});

const getResolvedOffset = (
  offset: OffsetEnum,
  side: SideEnum,
  instrumentType: InstrumentTypeEnum,
) => {
  if (shotable(instrumentType)) {
    if (offset !== undefined) {
      return offset;
    }
  }
  return side === 0 ? 0 : 1;
};

const shotable = (instrumentType: InstrumentTypeEnum): boolean => {
  return instrumentType
    ? ShotableInstrumentTypes.includes(instrumentType)
    : false;
};

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$globalBus.subscribe(
      (data: KfEvent.KfBusEvent) => {
        if (data.tag === 'makeOrder') {
          const { offset, side, volume, price, instrumentType, accountId } = (
            data as KfEvent.TriggerMakeOrder
          ).orderInput;

          const instrumentValue = buildInstrumentSelectOptionValue(
            (data as KfEvent.TriggerMakeOrder).orderInput,
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
            data as KfEvent.TriggerOrderBookUpdate
          ).orderInput;

          const instrumentValue = buildInstrumentSelectOptionValue(
            (data as KfEvent.TriggerOrderBookUpdate).orderInput,
          );

          if (!formState.value.instrument) {
            formState.value.instrument = instrumentValue;
            formState.value.instrument_type = +instrumentType;
          }

          if (!!price && !Number.isNaN(price) && +price !== 0) {
            formState.value.limit_price = +Number(price).toFixed(4);
          }

          if (
            !!volume &&
            !Number.isNaN(Number(volume)) &&
            BigInt(volume) !== BigInt(0)
          ) {
            formState.value.volume = +Number(volume).toFixed(0);
          }

          formState.value.side = +side;
        }
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(
  () => currentGlobalKfLocation.value,
  (newVal) => {
    if (newVal?.category === 'td') {
      formState.value.account_id = getIdByKfLocation(newVal);
    } else {
      formState.value.account_id = '';
    }
  },
);

watch(
  () => formState.value.instrument,
  (newVal) => {
    if (
      !formState.value.account_id &&
      currentGlobalKfLocation.value?.category !== 'td' &&
      instrumentKeyAccountsMap.value[newVal] &&
      instrumentKeyAccountsMap.value[newVal].length
    ) {
      formState.value.account_id = instrumentKeyAccountsMap.value[newVal][0];
    }

    if (!instrumentResolved.value) {
      return;
    }

    subscribeAllInstrumentByAppStates(
      processStatusData.value,
      appStates.value,
      mdExtTypeMap.value,
      [instrumentResolved.value],
    );
    triggerOrderBook(instrumentResolved.value);
    makeOrderInstrumentType.value = instrumentResolved.value.instrumentType;
  },
);

watch(
  () => formState.value.side,
  (newVal) => {
    formState.value.offset = getResolvedOffset(
      formState.value.offset,
      newVal,
      instrumentResolved.value?.instrumentType,
    );
  },
);

// 下单操作
function placeOrder(
  orderInput: KungfuApi.MakeOrderInput,
  globalKfLocation: KungfuApi.KfLocation,
  tdProcessId: string,
): Promise<bigint> {
  return makeOrderByOrderInput(
    window.watcher,
    orderInput,
    globalKfLocation,
    tdProcessId.toAccountId(),
  );
}

function initOrderInputData(): Promise<KungfuApi.MakeOrderInput> {
  if (!instrumentResolved.value) {
    return Promise.reject(new Error(t('instrument_error')));
  }

  const { exchangeId, instrumentId, instrumentType } = instrumentResolved.value;
  const { limit_price, volume, price_type, side, offset, hedge_flag, is_swap } =
    formState.value;

  const makeOrderInput: KungfuApi.MakeOrderInput = {
    instrument_id: instrumentId,
    instrument_type: +instrumentType,
    exchange_id: exchangeId,
    limit_price: +limit_price,
    volume: +volume,
    price_type: +price_type,
    side: +side,
    offset: getResolvedOffset(offset, side, instrumentType),
    hedge_flag: +(hedge_flag || 0),
    is_swap: !!is_swap,
  };

  return Promise.resolve(makeOrderInput);
}

function handleResetMakeOrderForm(): void {
  const initFormState = initFormStateByConfig(configSettings.value, {});

  Object.keys(initFormState).forEach((key) => {
    formState.value[key] = initFormState[key];
  });

  nextTick().then(() => {
    formRef.value.clearValidate();
  });
}

// 拆单
async function handleApartOrder(): Promise<void> {
  try {
    await formRef.value.validate();
    const makeOrderInput: KungfuApi.MakeOrderInput = await initOrderInputData();
    await showCloseModal(makeOrderInput);
    await confirmFatFingerModal(makeOrderInput);

    isShowConfirmModal.value = true;
    curOrderVolume.value = Number(makeOrderInput.volume);
    curOrderType.value = makeOrderInput.instrument_type;
  } catch (e) {
    if ((<Error>e).message) {
      error((<Error>e).message);
    }
  }
}

// 拆单弹窗确认回调
async function handleApartedConfirm(volumeList: number[]): Promise<void> {
  try {
    if (!makeOrderData.value || !currentGlobalKfLocation.value) return;

    const tdProcessId = await confirmOrderPlace(
      makeOrderData.value,
      volumeList.length,
    );
    const apartOrderInput: KungfuApi.MakeOrderInput = makeOrderData.value;

    Promise.all(
      volumeList.map((volume) => {
        apartOrderInput.volume = volume;
        return placeOrder(
          apartOrderInput as KungfuApi.MakeOrderInput,
          currentGlobalKfLocation.value as KungfuApi.KfLocation,
          tdProcessId,
        );
      }),
    );
  } catch (e) {
    if ((<Error>e).message) {
      error((<Error>e).message);
    }
  }
}

function confirmFatFingerModal(
  makeOrderInput: KungfuApi.MakeOrderInput,
): Promise<void> {
  const warnningMessage = dealFatFingerMessage(makeOrderInput);
  if (warnningMessage !== '') {
    return confirmModal(
      t('warning'),
      warnningMessage,
      t('tradingConfig.Continue'),
    );
  } else {
    return Promise.resolve();
  }
}

function dealFatFingerMessage(
  makeOrderInput: KungfuApi.MakeOrderInput,
): string {
  if (!instrumentResolved.value) {
    return '';
  }

  const fatFingerRange = +globalSetting.value?.trade?.fatFinger || 0;

  const { exchangeId, instrumentId } = instrumentResolved.value;
  const ukey = hashInstrumentUKey(instrumentId, exchangeId);

  const { limit_price: price, side } = makeOrderInput;
  const lastPrice = window.watcher.ledger.Quote[ukey]?.last_price;

  const fatFingerBuyRate =
    fatFingerRange === 0 ? 100 : (100 + fatFingerRange) / 100;
  const fatFingerSellRate =
    fatFingerRange === 0 ? 0 : (100 - fatFingerRange) / 100;

  if (SideEnum.Buy == side && price > lastPrice * fatFingerBuyRate) {
    return t('tradingConfig.fat_finger_buy_modal', {
      price: price,
      warningLine: (lastPrice * fatFingerBuyRate).toFixed(4),
      fatFinger: fatFingerRange,
    });
  }

  if (SideEnum.Sell == side && price < lastPrice * fatFingerSellRate) {
    return t('tradingConfig.fat_finger_sell_modal', {
      price: price,
      warningLine: (lastPrice * fatFingerSellRate).toFixed(4),
      fatFinger: fatFingerRange,
    });
  }

  return '';
}

async function confirmOrderPlace(
  makeOrderInput: KungfuApi.MakeOrderInput,
  orderCount: number = 1,
): Promise<string> {
  if (!currentGlobalKfLocation.value || !window.watcher) {
    return Promise.reject(new Error(t('location_error')));
  }

  const { account_id } = formState.value;
  const tdProcessId =
    currentGlobalKfLocation.value?.category === 'td'
      ? getProcessIdByKfLocation(currentGlobalKfLocation.value)
      : `td_${account_id.toString()}`;

  if (processStatusData.value[tdProcessId] !== 'online') {
    return Promise.reject(
      new Error(t('tradingConfig.start_process', { process: tdProcessId })),
    );
  }

  await confirmModal(
    t('tradingConfig.place_confirm'),
    dealOrderPlaceVNode(makeOrderInput, orderCount, false),
  );

  return Promise.resolve(tdProcessId);
}

// 下单
async function handleMakeOrder(): Promise<void> {
  try {
    if (!currentGlobalKfLocation.value) return;

    await formRef.value.validate();
    const makeOrderInput: KungfuApi.MakeOrderInput = await initOrderInputData();
    await showCloseModal(makeOrderInput);
    await confirmFatFingerModal(makeOrderInput);

    const tdProcessId = await confirmOrderPlace(makeOrderInput);
    await placeOrder(
      makeOrderInput,
      currentGlobalKfLocation.value,
      tdProcessId,
    );
  } catch (e) {
    if ((<Error>e).message) {
      error((<Error>e).message);
    }
  }
}

// 展示平仓弹窗
function showCloseModal(
  makeOrderInput: KungfuApi.MakeOrderInput,
): Promise<void> {
  if (!currentPosition.value) return Promise.resolve();

  const closeRange = +globalSetting.value?.trade?.close || 100;

  if (
    closeModalConditions(
      closeRange,
      makeOrderInput,
      Number(currentPosition.value?.volume || 0),
    )
  ) {
    return confirmModal(t('prompt'), t('tradingConfig.close_all'));
  }

  return Promise.resolve();
}

// 触发平仓弹窗条件
function closeModalConditions(
  closeRange: number,
  orderInput: KungfuApi.MakeOrderInput,
  positionVolume: number,
): boolean {
  const makeOrderInput = dealStockOffset(orderInput);
  const { offset } = makeOrderInput;

  if (offset === OffsetEnum.Open) {
    return false;
  }

  return makeOrderInput.volume >= positionVolume * (closeRange / 100);
}

const { handleOpenSetTradingTaskModal } = useTradingTask();
async function handleOpenTradingTaskConfigModal(
  kfExtConfig: KungfuApi.KfExtConfig,
) {
  try {
    if (!currentGlobalKfLocation.value) return;
    await formRef.value.validate();
    const taskInitValue = transformOrderInputToExtConfigForm(
      formState.value,
      configSettings.value,
      kfExtConfig.settings,
    );
    handleOpenSetTradingTaskModal('add', kfExtConfig.key, taskInitValue);
  } catch (e) {
    if ((<Error>e).message) {
      error((<Error>e).message);
    }
  }
}

const dealStringToNumber = (tar: string) =>
  Number.isNaN(Number(tar)) ? 0 : Number(tar);

let lastPercentSetVolume = 0;
const handlePercentChange = (target: number) => {
  const { side, offset } = formState.value;

  const curOffset = getResolvedOffset(
    offset,
    side,
    instrumentResolved.value?.instrumentType,
  );

  const targetPercent = target / 100;

  let targetVolume;
  if (curOffset === OffsetEnum.Open) {
    const availMoney = dealStringToNumber(currentAvailMoney.value);
    const allVolume = availMoney / currentPrice.value;
    targetVolume = allVolume * targetPercent;
  } else if (curOffset === OffsetEnum.Close) {
    const availPosVolume = dealStringToNumber(currentAvailPosVolume.value);
    targetVolume = availPosVolume * targetPercent;
  } else if (curOffset === OffsetEnum.CloseToday) {
    const availPosVolume = currentPosition.value?.volume;
    targetVolume = Number(availPosVolume) * targetPercent;
  } else if (curOffset === OffsetEnum.CloseYest) {
    const availPosVolume = currentPosition.value?.yesterday_volume;
    targetVolume = Number(availPosVolume) * targetPercent;
  }

  formState.value.volume = ~~targetVolume;
  if (targetVolume) {
    currentPercent.value = target;
    lastPercentSetVolume = ~~targetVolume;
  }
};

watch(
  () => formState.value.volume,
  (newVal) => {
    if (newVal !== lastPercentSetVolume) {
      currentPercent.value = 0;
    }
  },
);
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
            {{ $t('tradingConfig.reset_order') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <div class="make-order__warp">
        <div class="make-order-content">
          <div class="make-order-form__warp">
            <KfConfigSettingsForm
              ref="formRef"
              v-model:formState="formState"
              :configSettings="configSettings"
              changeType="add"
              :label-col="LABEL_COL"
              :wrapper-col="WRAPPER_COL"
              :rules="rules"
            ></KfConfigSettingsForm>
            <div class="percent-group__wrap">
              <a-col :span="LABEL_COL + WRAPPER_COL">
                <a-button
                  v-for="percent in percentList"
                  :class="{
                    'percent-button': true,
                    'percent-button-active': currentPercent === percent,
                  }"
                  :key="percent"
                  size="small"
                  ghost
                  @click="
                    currentPercent !== percent && handlePercentChange(percent)
                  "
                >
                  {{ `${percent}%` }}
                </a-button>
              </a-col>
            </div>
            <template v-if="isAccountOrInstrumentConfirmed">
              <div class="make-order-position">
                <a-col :span="LABEL_COL" class="position-label">
                  {{
                    showAmountOrPosition === 'amount'
                      ? $t('可用资金')
                      : $t('可用仓位')
                  }}
                </a-col>
                <a-col :span="WRAPPER_COL" class="position-value">
                  {{
                    showAmountOrPosition === 'amount'
                      ? currentAvailMoney
                      : currentAvailPosVolume
                  }}
                </a-col>
              </div>
              <div class="make-order-position">
                <a-col :span="LABEL_COL" class="position-label">
                  {{
                    shotable(instrumentResolved?.instrumentType)
                      ? formState.offset === OffsetEnum.Open
                        ? t('保证金占用')
                        : t('保证金返还')
                      : $t('交易金额')
                  }}
                </a-col>
                <a-col :span="WRAPPER_COL" class="position-value">
                  {{ currentTradeAmount }}
                </a-col>
              </div>
              <div class="make-order-position">
                <a-col :span="LABEL_COL" class="position-label">
                  {{
                    showAmountOrPosition === 'amount'
                      ? $t('剩余资金')
                      : $t('剩余仓位')
                  }}
                </a-col>
                <a-col :span="WRAPPER_COL" class="position-value">
                  {{
                    showAmountOrPosition === 'amount'
                      ? currentResidueMoney
                      : currentResiduePosVolume
                  }}
                </a-col>
              </div>
            </template>
          </div>
        </div>
        <div class="make-order-btns">
          <a-button class="make-order" @click="handleMakeOrder">
            {{ $t('tradingConfig.place_order') }}
          </a-button>
          <a-button @click="handleApartOrder">
            {{ $t('tradingConfig.apart_order') }}
          </a-button>
          <a-button
            v-for="item in availTradingTaskExtensionList"
            @click="handleOpenTradingTaskConfigModal(item)"
          >
            {{ item.name }}
          </a-button>
        </div>
      </div>
    </KfDashboard>
    <OrderConfirmModal
      v-if="isShowConfirmModal && curOrderType"
      v-model:visible="isShowConfirmModal"
      :curOrderVolume="curOrderVolume"
      :curOrderType="curOrderType"
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

    .make-order-content {
      flex: 1;
      height: 100%;
      display: flex;
      flex-direction: column;
      justify-content: flex-start;
    }

    .make-order-form__warp {
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

      .percent-group__wrap {
        margin: auto;
        padding-right: 16px;
        padding-left: 8px;
        box-sizing: border-box;

        .ant-col {
          margin: auto;
        }

        .percent-button {
          margin: 0px 8px 8px 0px;
          color: @border-color-base;
          border-color: @border-color-base;
        }

        .percent-button-active {
          color: @primary-color;
          border-color: @primary-color;
        }
      }
    }

    .make-order-position {
      display: flex;
      line-height: 1;
      font-size: 12px;
      color: @text-color-secondary;
      font-weight: bold;
      margin: 8px 0px;

      .position-label {
        padding-right: 8px;
        text-align: right;
      }

      .position-value {
        font-weight: bold;
      }

      &:first-child {
        margin-top: 8px;
      }
    }

    .make-order-btns {
      width: 40px;
      height: 100%;
      display: flex;
      flex-direction: column;

      .ant-btn {
        height: 26%;
        text-align: center;
        word-break: break-word;
        word-wrap: unset;
        white-space: normal;
        flex: 1;
        margin-bottom: 8px;

        &:last-child {
          margin-bottom: 0px;
        }
      }
      .make-order {
        height: 72%;
        flex: 4;
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
