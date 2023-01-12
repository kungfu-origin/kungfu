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
          <a-checkbox
            v-model:checked="isWithoutSuspensionInstrument"
            @change="handleFilterChange"
            size="small"
          >
            {{ $t('BasketTrade.without_suspension') }}
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-checkbox
            v-model:checked="isWithoutUpLimitInstrument"
            size="small"
            @change="handleFilterChange"
          >
            {{ $t('BasketTrade.without_up_limit') }}
          </a-checkbox>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-checkbox
            v-model:checked="isWithoutLowLimitInstrument"
            size="small"
            @change="handleFilterChange"
          >
            {{ $t('BasketTrade.without_low_limit') }}
          </a-checkbox>
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
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <a-button
                type="link"
                size="small"
                @click.stop="handleSubscribeBasketInstrument(item)"
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
      style="width: 900px"
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
  getCurrentInstance,
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
  dealBasketInstrumentsToMap,
  getAllBasketInstruments,
  setAllBasketInstruments,
  useCurrentGlobalBasket,
  useMakeBasketOrderFormModal,
  useSubscribeBasketInstruments,
  useMakeOrCancelBasketOrder,
} from '../../../utils/basketTradeUtils';
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
  getKfLocationByProcessId,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { BasketVolumeTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { promiseMessageWrapper } from '../../../utils';
const { t } = VueI18n.global;

const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();
const { currentGlobalBasket, currentBasketData } = useCurrentGlobalBasket();
const { getInstrumentByIds } = useActiveInstruments();
const { handleShowMakeBasketOrderModal } = useMakeBasketOrderFormModal();
const { makeBasketOrder } = useMakeOrCancelBasketOrder();
const { mdExtTypeMap } = useExtConfigsRelated();
const { appStates, processStatusData } = useProcessStatusDetailData();
const { subscribeAllInstrumentByAppStates } = useInstruments();
useSubscribeBasketInstruments();
const {
  getQuoteByInstrument,
  isInstrumentUpLimit,
  isInstrumentLowLimit,
  isInstrumentSuspension,
} = useQuote();

const dataTableRef = ref();
const dataSelection = ref<KfTradingDataTableSelection>({});
const basketInstrumentDataLoaded = ref(false);
const isWithoutSuspensionInstrument = ref(false);
const isWithoutUpLimitInstrument = ref(false);
const isWithoutLowLimitInstrument = ref(false);

const basketInstrumentLastPriceSorter = (
  a: KungfuApi.BasketInstrumentResolved,
  b: KungfuApi.BasketInstrumentResolved,
) => {
  return (
    (getQuoteByInstrument(a)?.last_price ?? 0) -
    (getQuoteByInstrument(b)?.last_price ?? 0)
  );
};
const columns = getColumns(
  currentGlobalBasket,
  basketInstrumentLastPriceSorter,
);

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
      dataTableRef.value && dataTableRef.value.handleSelectAll(true);
    })
    .catch((err) => {
      console.log(err);
    });
});

watch(
  () => currentGlobalBasket.value,
  (newGlobalBasket) => {
    if (!newGlobalBasket) return;
    handleGetAllBasketInstruments()
      .then(() => {
        basketInstrumentDataLoaded.value = true;

        dataTableRef.value && dataTableRef.value.handleSelectAll(true);
      })
      .catch((err) => {
        console.log(err);
      });
  },
);

function handleGetAllBasketInstruments() {
  if (!currentGlobalBasket.value) return Promise.resolve();

  return getAllBasketInstruments().then((basketInstruments) => {
    if (!currentGlobalBasket.value) return;

    basketInstrumentsResolvedMap.value = dealBasketInstrumentsToMap(
      currentGlobalBasket.value,
      basketInstruments,
    );
  });
}

function handleSetAllBasketInstruments(
  basketInstruments: KungfuApi.BasketInstrument[],
) {
  const promise = setAllBasketInstruments(basketInstruments).then((res) => {
    if (res) {
      return handleGetAllBasketInstruments();
    } else {
      return Promise.reject();
    }
  });

  promiseMessageWrapper(promise, { errorTextByError: true });
}

function handleSubscribeBasketInstrument(
  basketInstrument: KungfuApi.BasketInstrumentResolved,
) {
  const subscribePromise = subscribeAllInstrumentByAppStates(
    processStatusData.value,
    appStates.value,
    mdExtTypeMap.value,
    [basketInstrument],
  );

  promiseMessageWrapper(subscribePromise, {
    errorTextByError: true,
  });
}

function handlePlaceBasketOrder(formState) {
  if (!currentGlobalBasket.value) return;

  const accountLocation = getKfLocationByProcessId(`td_${formState.accountId}`);
  if (!accountLocation) return;

  if (!dataTableRef.value) return;

  const selectBasketInstruments = Object.values(
    dataTableRef.value.selectedRowsMap || {},
  ) as unknown as KungfuApi.BasketInstrumentResolved[];

  const basketOrderInput: KungfuApi.BasketOrderInput = {
    side: +formState.side,
    offset: +formState.offset,
    price_type: +formState.priceType,
    price_level: +formState.priceLevel,
    price_offset: +formState.priceOffset,
    volume: BigInt(currentGlobalBasket.value.total_volume),
  };

  makeBasketOrder(
    window.watcher,
    currentGlobalBasket.value,
    basketOrderInput,
    selectBasketInstruments,
    accountLocation,
  ).then(() => {
    app?.proxy?.$globalBus.next({
      tag: 'update:makeBasketOrder',
    });
  });
}

function handleOpenSetBasketInstrumentModal() {
  if (!currentGlobalBasket.value) return Promise.reject();

  setBasketInstrumentConfigPayload.value.type = 'update';
  setBasketInstrumentConfigPayload.value.config = {
    type: [],
    name: t('BasketTrade.basket_instrument'),
    category: BASKET_CATEGORYS.SETTING,
    key: BASKET_CATEGORYS.SETTING,
    extPath: '',
    settings: getSetBasketInstrumentFormSettings(
      currentGlobalBasket.value.volume_type,
    ).value,
  };

  setBasketInstrumentConfigPayload.value.initValue = {
    basketInstruments: toRaw(basketInstrumentsResolved.value).map((item) => {
      return {
        ...item,
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
    targetBasketInstrument.id
  ];

  return handleSetAllBasketInstruments(
    Object.values(allBasketInstruments)
      .map((map) => Object.values(map))
      .flat(1),
  );
}

function handleFilterChange() {
  const suspensionBasketInstrumentKeys: string[] = [];
  const upLimitBasketInstrumentKeys: string[] = [];
  const lowLimitBasketInstrumentKeys: string[] = [];

  basketInstrumentsResolved.value.forEach((basketInstrument) => {
    if (isInstrumentSuspension(basketInstrument)) {
      suspensionBasketInstrumentKeys.push(basketInstrument.id);
    }
    if (isInstrumentUpLimit(basketInstrument)) {
      upLimitBasketInstrumentKeys.push(basketInstrument.id);
    }
    if (isInstrumentLowLimit(basketInstrument)) {
      lowLimitBasketInstrumentKeys.push(basketInstrument.id);
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
        basketInstrumentsResolvedMap.value[currentGlobalBasket.value?.id][key],
      );

    return selection;
  }, {} as KfTradingDataTableSelection);
}
</script>

<style lang="less">
.kf-basket-trade-setting__warp {
  height: 100%;
}
</style>
