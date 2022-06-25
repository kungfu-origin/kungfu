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
import { getConfigSettings } from './config';
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
  DirectionEnum,
  InstrumentTypeEnum,
  OffsetEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  useAssets,
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealKfNumber,
  dealKfPrice,
  dealTradingData,
  getExtConfigList,
  getProcessIdByKfLocation,
  initFormStateByConfig,
  isTdStrategyCategory,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import OrderConfirmModal from './OrderConfirmModal.vue';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { useTradingTask } from '../tradingTask/utils';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { storeToRefs } from 'pinia';
import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

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
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { getExtraCategoryData } = useExtraCategory();

const { getAssetsByKfConfig } = useAssets();

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
const curOrderType = ref<InstrumentTypeEnum>(InstrumentTypeEnum.unknown);

const instrumentResolved = computed(() => {
  const { instrument } = formState.value;
  return instrument
    ? transformSearchInstrumentResultToInstrument(instrument)
    : null;
});

const makeOrderData = computed(() => {
  if (!instrumentResolved.value) {
    return null;
  }

  const { exchangeId, instrumentId, instrumentType } = instrumentResolved.value;

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
    offset: getResolvedOffset(offset, side, instrumentType),
    hedge_flag: +(hedge_flag || 0),
  };
  return makeOrderInput;
});

const curPositionList = ref<KungfuApi.Position[]>();

const isCurrentCategoryIsTd = computed(
  () => currentGlobalKfLocation.value?.category === 'td',
);

const isAccountOrInstrumentConfirmed = computed(() => {
  if (formState.value?.side === SideEnum.Buy) {
    return isCurrentCategoryIsTd.value ? true : !!formState.value.account_id;
  } else if (formState.value.side === SideEnum.Sell) {
    return isCurrentCategoryIsTd.value
      ? !!formState.value.instrument
      : formState.value.account_id && formState.value.instrument;
  }
});

const currentPosition = computed(() => {
  if (!curPositionList.value?.length || !instrumentResolved.value) return null;

  const { exchangeId, instrumentId, instrumentType } = instrumentResolved.value;
  const targetPositionList: KungfuApi.Position[] = curPositionList.value.filter(
    (position) =>
      position.exchange_id === exchangeId &&
      position.instrument_id === instrumentId &&
      position.instrument_type === instrumentType,
  );

  if (targetPositionList && targetPositionList.length) {
    const { side, offset } = formState.value;

    const targetPositionWithLongDirection = targetPositionList.filter(
      (item) => item.direction === DirectionEnum.Long,
    )[0];
    const targetPositionWithShortDirection = targetPositionList.filter(
      (item) => item.direction === DirectionEnum.Short,
    )[0];

    if (side === SideEnum.Buy) {
      if (offset === OffsetEnum.Open) {
        return targetPositionWithLongDirection;
      } else {
        return targetPositionWithShortDirection;
      }
    } else if (side === SideEnum.Sell) {
      if (offset === OffsetEnum.Open) {
        return targetPositionWithShortDirection;
      } else {
        return targetPositionWithLongDirection;
      }
    }
  }

  return null;
});

const showAmountOrPosition = computed(() => {
  const { offset } = formState.value;

  return offset === OffsetEnum.Open ? 'amount' : 'position';
});

const currentAvailMoney = computed(() => {
  if (!currentGlobalKfLocation.value) return '--';

  if (isCurrentCategoryIsTd.value) {
    const avail = getAssetsByKfConfig(currentGlobalKfLocation.value).avail;

    return dealKfPrice(avail);
  } else {
    if (formState.value?.account_id) {
      const { source, id } = formState.value.account_id.parseSourceAccountId();

      const avail = getAssetsByKfConfig({
        category: 'td',
        group: source,
        name: id,
        mode: 'live',
      }).avail;

      return dealKfPrice(avail);
    }
  }

  return '--';
});

const currentAvailPosVolume = computed(() => {
  if (!instrumentResolved.value) return '--';

  const { instrumentType } = instrumentResolved.value;
  const { offset } = formState.value;

  if (currentPosition.value) {
    const { yesterday_volume, volume } = currentPosition.value;

    if (shotable(instrumentType)) {
      if (offset === OffsetEnum.CloseYest) {
        return dealKfNumber(yesterday_volume);
      } else if (offset === OffsetEnum.CloseToday) {
        return dealKfNumber(volume - yesterday_volume);
      } else {
        return dealKfNumber(volume);
      }
    } else {
      return dealKfNumber(yesterday_volume);
    }
  }

  return '0';
});

function getFutureInstrumentTradeAmount(
  currentPrice: number,
  volume: number,
  instrument,
  direction: DirectionEnum,
): number | null {
  if (!instrument) return null;

  const { exchangeId, instrumentId } = instrument;
  const instrumentKey = hashInstrumentUKey(instrumentId, exchangeId);
  const { contract_multiplier, long_margin_ratio, short_margin_ratio } = window
    .watcher.ledger.Instrument[instrumentKey] as KungfuApi.Instrument;

  if (direction === DirectionEnum.Long) {
    return currentPrice * volume * contract_multiplier * long_margin_ratio;
  } else if (direction === DirectionEnum.Short) {
    return currentPrice * volume * contract_multiplier * short_margin_ratio;
  }

  return null;
}

function dealTradeAmount(preNumber: number | null) {
  return !Number(preNumber) ? '--' : dealKfPrice(preNumber);
}

function getTradeAmount(
  currentPrice: number,
  volume: number,
  currentInstrument?: KungfuApi.InstrumentResolved,
  currentPosition?: KungfuApi.PositionResolved,
): string | null {
  const instrumentType = currentInstrument?.instrumentType;

  if (instrumentType) {
    if (instrumentType === InstrumentTypeEnum.future) {
      const instrumentTradeAmount = getFutureInstrumentTradeAmount(
        currentPrice,
        volume,
        currentInstrument,
        currentPosition?.direction,
      );
      return dealTradeAmount(instrumentTradeAmount);
    } else if (instrumentType === InstrumentTypeEnum.stock) {
      return dealTradeAmount(currentPrice * volume);
    }
  } else {
    return dealTradeAmount(currentPrice * volume);
  }

  return null;
}

const currentTradeAmount = computed(() => {
  const { price_type, limit_price, volume, side, account_id } = formState.value;

  let currentPrice;
  if (price_type === PriceTypeEnum.Limit) {
    currentPrice = limit_price;
  } else if (price_type === PriceTypeEnum.Market) {
    currentPrice = currentPosition.value?.last_price;
  }

  if (side === SideEnum.Buy) {
    return getTradeAmount(currentPrice, volume);
  } else if (side === SideEnum.Sell) {
    if (
      instrumentResolved.value &&
      (currentGlobalKfLocation.value?.category === 'td' || account_id)
    ) {
      return getTradeAmount(
        currentPrice,
        volume,
        instrumentResolved.value.instrumentType,
      );
    }
  }

  return '--';
});

const currentResidueMoney = computed(() => {
  const { side } = formState.value;
  if (currentAvailMoney.value !== '--') {
    if (currentTradeAmount.value !== '--') {
      if (side === SideEnum.Buy) {
        return dealKfPrice(
          Number(currentAvailMoney.value) - Number(currentTradeAmount.value),
        );
      } else if (side === SideEnum.Sell) {
        return dealKfPrice(
          Number(currentAvailMoney.value) + Number(currentTradeAmount.value),
        );
      }

      return '--';
    } else {
      return currentAvailMoney.value;
    }
  } else {
    return '--';
  }
});

const currentResiduePosVolume = computed(() => {
  const { volume, offset } = formState.value;
  if (currentAvailPosVolume.value !== '--') {
    if (volume && volume > 0) {
      if (offset === OffsetEnum.Open) {
        return dealKfNumber(
          Number(currentAvailPosVolume.value) + Number(volume),
        );
      } else {
        return dealKfNumber(
          Number(currentAvailPosVolume.value) - Number(volume),
        );
      }
    } else {
      return currentAvailPosVolume.value;
    }
  } else {
    return '--';
  }
});

const availTradingTaskExtensionList = computed(() => {
  return getExtConfigList(extConfigs.value, 'strategy').filter((item) => {
    return uiExtConfigs.value[item.key]?.position === 'make_order';
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
      updatePositionList();
    });

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

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

    updatePositionList();
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

// 更新持仓列表
function updatePositionList(): void {
  if (currentGlobalKfLocation.value === null) {
    return;
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

  curPositionList.value = positions;
}

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
    offset: getResolvedOffset(offset, side, instrumentType),
    hedge_flag: +(hedge_flag || 0),
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
    if (e.message) {
      error(e.message);
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
    error(e);
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
    dealOrderPlaceVNode(makeOrderInput, orderCount),
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
    if (e.message) {
      error(e.message);
    }
  }
}

// 展示平仓弹窗
function showCloseModal(
  makeOrderInput: KungfuApi.MakeOrderInput,
): Promise<void> {
  if (!currentPosition.value) return Promise.resolve();

  updatePositionList();
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
    if (e.message) {
      error(e.message);
    }
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
              :label-col="5"
              :wrapper-col="14"
            ></KfConfigSettingsForm>
            <template v-if="isAccountOrInstrumentConfirmed">
              <div class="make-order-position">
                <div class="position-label">
                  {{
                    showAmountOrPosition === 'amount'
                      ? $t('可用资金')
                      : $t('可用仓位')
                  }}:&nbsp
                </div>
                <div class="position-value">
                  {{
                    showAmountOrPosition === 'amount'
                      ? currentAvailMoney
                      : currentAvailPosVolume
                  }}
                </div>
              </div>
              <div class="make-order-position">
                <div class="position-label">
                  {{
                    shotable(instrumentResolved?.instrumentType)
                      ? $t('保证金占用')
                      : $t('交易金额')
                  }}:&nbsp
                </div>
                <div class="position-value">
                  {{ currentTradeAmount }}
                </div>
              </div>
              <div class="make-order-position">
                <div class="position-label">
                  {{
                    showAmountOrPosition === 'amount'
                      ? $t('剩余资金')
                      : $t('剩余仓位')
                  }}:&nbsp
                </div>
                <div class="position-value">
                  {{
                    showAmountOrPosition === 'amount'
                      ? currentResidueMoney
                      : currentResiduePosVolume
                  }}
                </div>
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
      line-height: 1;
      font-size: 12px;
      color: @text-color;
      font-weight: bold;
      margin-bottom: 8px;
      margin-left: 10px;

      &:last-child {
        margin-bottom: 0;
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
