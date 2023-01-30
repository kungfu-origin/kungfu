<template>
  <div class="kf-basket-trade-setting__warp kf-translateZ">
    <KfDashboard @board-size-change="handleBodySizeChange">
      <template #title>
        <span v-if="currentGlobalBasket">
          <a-tag
            v-if="currentGlobalBasket"
            :color="currentBasketData?.color || 'default'"
          >
            {{ currentBasketData?.name }}
          </a-tag>
          <span v-if="currentGlobalBasket" class="name">
            {{ currentGlobalBasket?.name }}
          </span>
        </span>
      </template>
      <template #header>
        <!-- <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem> -->
        <KfDashboardItem>
          <a-checkbox v-model:checked="isWithoutUpLimitInstrument" size="small">
            {{ $t('BasketTrade.without_up_limit') }}
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-checkbox
            v-model:checked="isWithoutLowLimitInstrument"
            size="small"
          >
            {{ $t('BasketTrade.without_low_limit') }}
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            @click.stop="
              handleSubscribeBasketInstruments(basketInstrumentsResolved)
            "
          >
            {{ $t('BasketTrade.subscribe') }}
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleOpenSetBasketInstrumentModal">
            <template #icon>
              <SettingOutlined />
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="
              handleShowMakeBasketOrderModal(
                `${$t('BasketTrade.place_order')} ${currentGlobalBasket?.name}`,
                getMakeBasketOrderConfigSettings(),
                handlePlaceBasketOrder,
              )
            "
          >
            {{ $t('tradingConfig.place_order') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <KfTradingDataTable
        v-if="basketInstrumentDataLoaded"
        ref="dataTableRef"
        :columns="columns"
        :data-source="basketInstrumentsResolved"
        key-field="id"
        :selectable="true"
        :seletion="dataSelection"
      >
        <template
          #default="{
            item,
            column,
          }: {
            item: KungfuApi.BasketInstrumentResolved,
            column: KfTradingDataTableHeaderConfig,
          }"
        >
          <template v-if="column.dataIndex === 'last_price'">
            <KfBlinkNum
              :num="dealKfPrice(getQuoteByInstrument(item)?.last_price)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'rate'">
            {{ `${item['rate']}%` }}
          </template>
          <template v-else-if="column.dataIndex === 'direction'">
            <span :class="`color-${dealDirection(item.direction).color}`">
              {{ dealDirection(item.direction).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <a-button
                type="link"
                size="small"
                @click.stop="handleSubscribeBasketInstruments([item])"
              >
                {{ $t('BasketTrade.subscribe') }}
              </a-button>
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveBasketInstrument(item)"
              />
            </div>
          </template>
        </template>
      </KfTradingDataTable>
    </KfDashboard>
    <KfSetByConfigModal
      v-if="addBasketInstrumentModalVisble"
      v-model:visible="addBasketInstrumentModalVisble"
      :width="1010"
      :label-col="4"
      :wrapper-col="17"
      :payload="setBasketInstrumentConfigPayload"
      @confirm="
        ({ formState }) => handleConfirmAddUpdateBasketInstrument(formState)
      "
    ></KfSetByConfigModal>
  </div>
</template>

<script lang="ts" setup>
import {
  ref,
  computed,
  onMounted,
  toRaw,
  watch,
  watchEffect,
  getCurrentInstance,
  nextTick,
} from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import { SettingOutlined, DeleteOutlined } from '@ant-design/icons-vue';

import {
  buildInstrumentSelectOptionValue,
  useDashboardBodySize,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  buildBasketMapKeyAndLocation,
  dealBasketInstrumentsToMap,
  useCurrentGlobalBasket,
  useMakeBasketOrderFormModal,
  useSubscribeBasketInstruments,
  useMakeOrCancelBasketOrder,
} from '../../../utils/basketTradeUtils';
import {
  getAllBasketInstruments,
  setAllBasketInstruments,
  setAllBaskets,
} from '@kungfu-trader/kungfu-js-api/actions';
import {
  useActiveInstruments,
  useExtConfigsRelated,
  useInstruments,
  useProcessStatusDetailData,
  useQuote,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { getColumns, getSetBasketInstrumentFormSettings } from './config';
import { BASKET_CATEGORYS } from '../../../config';
import { getMakeBasketOrderConfigSettings } from '../../../config/makeBasketOrderFormConfig';
import {
  dealKfPrice,
  dealDirection,
  getKfLocationByProcessId,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  BasketVolumeTypeEnum,
  DirectionEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { promiseMessageWrapper } from '../../../utils';
import { useBasketTradeStore } from '../../../store';
import { storeToRefs } from 'pinia';
const { t } = VueI18n.global;

const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();
const { basketsMap } = storeToRefs(useBasketTradeStore());
const { currentGlobalBasket, currentBasketData } = useCurrentGlobalBasket();
const { getInstrumentByIds } = useActiveInstruments();
const { handleShowMakeBasketOrderModal } = useMakeBasketOrderFormModal();
const { makeBasketOrder } = useMakeOrCancelBasketOrder();
const { mdExtTypeMap } = useExtConfigsRelated();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { subscribeAllInstrumentByAppStates } = useInstruments();
useSubscribeBasketInstruments();
const { getQuoteByInstrument, isInstrumentUpLimit, isInstrumentLowLimit } =
  useQuote();

const dataTableRef = ref();
const dataSelection = ref<KfTradingDataTableSelection>({});
const basketInstrumentDataLoaded = ref(false);
const isWithoutUpLimitInstrument = ref(false);
const isWithoutLowLimitInstrument = ref(false);

const basketInstrumentSorter = (
  getter: (bi: KungfuApi.BasketInstrumentResolved) => number,
) => {
  return (
    a: KungfuApi.BasketInstrumentResolved,
    b: KungfuApi.BasketInstrumentResolved,
  ) => {
    return (getter(a) ?? 0) - (getter(b) ?? 0);
  };
};

const columns = getColumns(currentGlobalBasket, {
  last_price: basketInstrumentSorter(
    (bi: KungfuApi.BasketInstrumentResolved) =>
      getQuoteByInstrument(bi)?.last_price ?? 0,
  ),
});

const basketInstrumentsResolvedMap = ref<
  Record<string, Record<string, KungfuApi.BasketInstrumentResolved>>
>({});
const basketInstrumentsResolved = computed(() =>
  currentGlobalBasket.value
    ? Object.values(
        basketInstrumentsResolvedMap.value[currentGlobalBasket.value.id] || {},
      )
    : [],
);
const addBasketInstrumentModalVisble = ref(false);

const setBasketInstrumentConfigPayload = ref<KungfuApi.SetKfConfigPayload>({
  type: 'add',
  title: t('BasketTrade.basket_instrument'),
  config: {} as KungfuApi.KfExtConfig,
});

onMounted(() => {
  handleGetAllBasketInstruments()
    .then(() => {
      basketInstrumentDataLoaded.value = true;
    })
    .catch((err) => {
      console.error(err);
    });
});

watch(
  () => currentGlobalBasket.value,
  (newGlobalBasket) => {
    if (!newGlobalBasket) return;
    handleGetAllBasketInstruments()
      .then(() => {
        basketInstrumentDataLoaded.value = true;
      })
      .catch((err) => {
        console.error(err);
      });
  },
);

watchEffect(() => {
  const suspensionBasketInstrumentKeys: string[] = [];
  const upLimitBasketInstrumentKeys: string[] = [];
  const lowLimitBasketInstrumentKeys: string[] = [];

  basketInstrumentsResolved.value.forEach((basketInstrument) => {
    if (
      isWithoutUpLimitInstrument.value &&
      isInstrumentUpLimit(basketInstrument)
    ) {
      upLimitBasketInstrumentKeys.push(basketInstrument.basketInstrumentId);
    }
    if (
      isWithoutUpLimitInstrument.value &&
      isInstrumentLowLimit(basketInstrument)
    ) {
      lowLimitBasketInstrumentKeys.push(basketInstrument.basketInstrumentId);
    }
  });

  const basketInstrumentsToDisabled = Array.from(
    new Set([
      ...suspensionBasketInstrumentKeys,
      ...upLimitBasketInstrumentKeys,
      ...lowLimitBasketInstrumentKeys,
    ]),
  );

  dataSelection.value = basketInstrumentsToDisabled.reduce((selection, key) => {
    selection[key] = {
      disabled: true,
    };

    currentGlobalBasket.value &&
      dataTableRef.value &&
      dataTableRef.value.handleSelectRow(
        false,
        basketInstrumentsResolvedMap.value[currentGlobalBasket.value.id][key],
      );

    return selection;
  }, {} as KfTradingDataTableSelection);
});

function handleGetAllBasketInstruments() {
  if (!currentGlobalBasket.value) return Promise.resolve();

  return getAllBasketInstruments().then((basketInstruments) => {
    if (!currentGlobalBasket.value) return;

    basketInstrumentsResolvedMap.value = dealBasketInstrumentsToMap(
      currentGlobalBasket.value,
      basketInstruments,
    );

    nextTick(() => {
      dataTableRef.value && dataTableRef.value.handleSelectAll(true);
    });
  });
}

function handleSetAllBasketInstruments(
  basketInstruments: KungfuApi.BasketInstrument[],
) {
  const promise = setAllBasketInstruments(basketInstruments).then((res) => {
    if (res) {
      return handleGetAllBasketInstruments().then(() => {
        return updateBasketTotalVolume(basketInstruments);
      });
    } else {
      return Promise.reject();
    }
  });

  return promiseMessageWrapper(promise, { errorTextByError: true });
}

function handleSubscribeBasketInstruments(
  basketInstruments: KungfuApi.BasketInstrumentResolved[],
) {
  const subscribePromise = subscribeAllInstrumentByAppStates(
    processStatusData.value,
    appStates.value,
    mdExtTypeMap.value,
    basketInstruments,
  );

  promiseMessageWrapper(subscribePromise, {
    errorTextByError: true,
  });
}

function handlePlaceBasketOrder(formState) {
  if (!currentGlobalBasket.value) return;

  const accountIds = formState.accountIds as string[];

  const accountLocations = accountIds
    .map((accountId) => getKfLocationByProcessId(`td_${accountId}`))
    .filter((item) => !!item) as KungfuApi.KfLocation[];

  if (!dataTableRef.value) return;

  const selectBasketInstruments = Object.values(
    dataTableRef.value.selectedRowsMap || {},
  ) as unknown as KungfuApi.BasketInstrumentResolved[];

  const basketOrderInput: KungfuApi.BasketOrderInput = {
    side: +formState.side,
    price_type: +formState.priceType,
    price_level: +formState.priceLevel,
    price_offset: +formState.priceOffset,
    volume: BigInt(currentGlobalBasket.value.total_amount),
  };

  makeBasketOrder(
    window.watcher,
    currentGlobalBasket.value,
    basketOrderInput,
    selectBasketInstruments,
    accountLocations,
    +formState.basketVolume,
  ).then(() => {
    app?.proxy?.$globalBus.next({
      tag: 'update:makeBasketOrder',
    });
  });
}

function updateBasketTotalVolume(
  basketInstruments: KungfuApi.BasketInstrument[],
) {
  if (!currentGlobalBasket.value) return Promise.resolve();

  if (currentGlobalBasket.value.volume_type === BasketVolumeTypeEnum.Quantity) {
    basketsMap.value[
      buildBasketMapKeyAndLocation(currentGlobalBasket.value).key
    ].total_amount = basketInstruments.reduce(
      (totalVolume, cur) => totalVolume + cur.volume,
      0n,
    );

    return setAllBaskets(Object.values(basketsMap.value)).then((res) => {
      if (res) {
        return Promise.resolve();
      } else {
        return Promise.reject();
      }
    });
  }

  return Promise.resolve();
}

function handleOpenSetBasketInstrumentModal() {
  if (!currentGlobalBasket.value) return Promise.reject();

  setBasketInstrumentConfigPayload.value.type = 'update';
  setBasketInstrumentConfigPayload.value.config = {
    type: [],
    name: t('BasketTrade.basket_instrument'),
    category: BASKET_CATEGORYS.INSTRUMENT,
    key: BASKET_CATEGORYS.INSTRUMENT,
    extPath: '',
    settings: getSetBasketInstrumentFormSettings(
      currentGlobalBasket.value.volume_type,
    ).value,
  };

  setBasketInstrumentConfigPayload.value.initValue = {
    basketInstruments: toRaw(basketInstrumentsResolved.value).map((item) => {
      return {
        ...item,
        direction: `${item.direction}`,
        volume: Number(item.volume),
        instrument: buildInstrumentSelectOptionValue(
          getInstrumentByIds(
            item.instrument_id,
            item.exchange_id,
            true,
          ) as KungfuApi.InstrumentResolved,
        ),
      };
    }),
  };

  addBasketInstrumentModalVisble.value = true;

  return Promise.resolve();
}

function handleConfirmAddUpdateBasketInstrument(
  formState: Record<string, KungfuApi.KfConfigValue>,
) {
  if (!currentGlobalBasket.value) return Promise.reject();

  const newBasketInstruments: KungfuApi.BasketInstrument[] = (
    formState.basketInstruments as Array<{
      instrument: string;
      direction: DirectionEnum;
      volume: bigint;
      rate: number;
    }>
  ).reduce((resolvedBasketInstruments, item) => {
    const instrumentResolved = transformSearchInstrumentResultToInstrument(
      item.instrument,
    );

    if (!instrumentResolved || !currentGlobalBasket.value)
      return resolvedBasketInstruments;

    const { instrumentId, instrumentType, exchangeId } = instrumentResolved;

    const isBasketVolumeByQuantity =
      currentGlobalBasket.value.volume_type === BasketVolumeTypeEnum.Quantity;

    resolvedBasketInstruments.push({
      basket_uid: currentGlobalBasket.value.id,
      instrument_id: instrumentId,
      exchange_id: exchangeId,
      instrument_type: instrumentType,
      direction: +item.direction,
      volume: isBasketVolumeByQuantity ? BigInt(item.volume) : BigInt(0),
      rate: isBasketVolumeByQuantity ? 0 : +item.rate,
    });

    return resolvedBasketInstruments;
  }, [] as KungfuApi.BasketInstrument[]);

  const newBasketInstrumentsMap = dealBasketInstrumentsToMap(
    currentGlobalBasket.value,
    newBasketInstruments,
  );

  const allBasketInstruments = Object.assign(
    toRaw(basketInstrumentsResolvedMap.value),
    newBasketInstrumentsMap,
  );

  return handleSetAllBasketInstruments(
    Object.values(allBasketInstruments)
      .map((map) => Object.values(map))
      .flat(1),
  );
}

function handleRemoveBasketInstrument(
  targetBasketInstrument: KungfuApi.BasketInstrumentResolved,
) {
  if (!currentGlobalBasket.value) return Promise.reject();

  const allBasketInstruments = Object.assign(
    {},
    toRaw(basketInstrumentsResolvedMap.value),
  );

  delete allBasketInstruments[currentGlobalBasket.value.id][
    targetBasketInstrument.basketInstrumentId
  ];

  return handleSetAllBasketInstruments(
    Object.values(allBasketInstruments)
      .map((map) => Object.values(map))
      .flat(1),
  );
}
</script>

<style lang="less">
.kf-basket-trade-setting__warp {
  height: 100%;
}
</style>
