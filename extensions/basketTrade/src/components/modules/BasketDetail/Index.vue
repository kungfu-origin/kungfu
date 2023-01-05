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
                $t('BasketTrade.place_order'),
                getMakeBasketOrderConfigSettings(),
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
      >
        <template
          #bodyCell="{
            item,
            column,
          }: {
            item: KungfuApi.BasketInstrumentResolved,
            column: KfTradingDataTableHeaderConfig,
          }"
        >
          <template v-if="column.dataIndex === 'rate'">
            {{ `${item['rate']}%` }}
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
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
import { ref, computed, onMounted, toRaw } from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import { SettingOutlined, DeleteOutlined } from '@ant-design/icons-vue';

import {
  buildInstrumentSelectOptionValue,
  messagePrompt,
  useDashboardBodySize,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  dealBasketInstrumentsToMap,
  getAllBasketInstruments,
  setAllBasketInstruments,
  useCurrentGlobalBasket,
  useMakeBasketOrder,
} from '../../../utils/basketTradeUtils';
import { useActiveInstruments } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { getColumns, getSetBasketInstrumentFormSettings } from './config';
import { BASKET_CATEGORYS } from '../../../config';
import { getMakeBasketOrderConfigSettings } from '../../../config/makeBasketOrderFormConfig';
import {
  getKfLocationByProcessId,
  transformSearchInstrumentResultToInstrument,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { BasketVolumeTypeEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { makeOrderByBasketTrade } from '@kungfu-trader/kungfu-js-api/kungfu';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const { handleBodySizeChange } = useDashboardBodySize();
const { currentGlobalBasket, currentBasketData } = useCurrentGlobalBasket();
const { getInstrumentByIds } = useActiveInstruments();
const { handleShowMakeBasketOrderModal } = useMakeBasketOrder({
  handleConfirmModal: handlePlaceBasketOrder,
});

const dataTableRef = ref();
const basketInstrumentDataLoaded = ref(false);
const columns = getColumns(currentGlobalBasket);

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
  handleGetAllBasketInstruments().then(() => {
    basketInstrumentDataLoaded.value = true;
  });
});

function handleGetAllBasketInstruments() {
  return getAllBasketInstruments().then((basketInstruments) => {
    basketInstrumentsResolvedMap.value =
      dealBasketInstrumentsToMap(basketInstruments);
  });
}

function handleSetAllBasketInstruments(
  basketInstruments: KungfuApi.BasketInstrument[],
) {
  return setAllBasketInstruments(basketInstruments)
    .then((res) => {
      if (res) {
        return handleGetAllBasketInstruments();
      } else {
        return Promise.reject();
      }
    })
    .then(() => {
      messagePrompt().success();
    })
    .catch((err) => {
      if (err instanceof Error) {
        err.message && console.error(err);
      }
      messagePrompt().error();
    });
}

function handlePlaceBasketOrder(formState) {
  if (!currentGlobalBasket.value) return;

  const accountLocation = getKfLocationByProcessId(`td_${formState.accountId}`);
  if (!accountLocation) return;
  console.log(dataTableRef.value);
  const selectBasketInstruments = Object.values(
    dataTableRef.value.selectedRowsMap,
  ) as KungfuApi.BasketInstrumentResolved[];

  const basketOrder: KungfuApi.BasketOrderInput = {
    side: +formState.side,
    offset: +formState.offset,
    price_type: +formState.priceType,
    price_level: +formState.priceLevel,
    price_offset: +formState.priceOffset,
    volume: BigInt(currentGlobalBasket.value.total_volume),
  };

  makeOrderByBasketTrade(
    window.watcher,
    currentGlobalBasket.value,
    basketOrder,
    selectBasketInstruments,
    accountLocation,
  ).then((orderIds) => {
    if (orderIds.length === selectBasketInstruments.length) {
      messagePrompt().success();
    }
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

  const newBasketInstrumentsMap =
    dealBasketInstrumentsToMap(newBasketInstruments);

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
</script>

<style lang="less">
.kf-basket-trade-setting__warp {
  height: 100%;
}
</style>
