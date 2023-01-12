<template>
  <div class="kf-orders__warp kf-translateZ">
    <KfDashboard @board-size-change="handleBodySizeChange">
      <template #title>
        <span v-if="currentGlobalBasket">
          <a-tag
            v-if="currentBasketData"
            :color="currentBasketData?.color || 'default'"
          >
            {{ currentBasketData?.name }}
          </a-tag>
          <span v-if="currentGlobalBasket" class="name">
            {{ currentGlobalBasket?.name }}
          </span>
        </span>
      </template>
      <div ref="tableRef" class="kf-table__warp">
        <KfTradingDataTable
          :columns="columns"
          :data-source="basketOrders"
          key-field="order_id"
          :custom-row-class="dealRowClassNameResolved"
          @click-cell="handleClickRow"
        >
          <template
            #default="{
              item,
              column,
            }: {
              item: KungfuApi.BasketOrderResolved,
              column: KfTradingDataTableHeaderConfig,
            }"
          >
            <template v-if="column.dataIndex === 'side'">
              <span :class="`color-${dealSide(item.side).color}`">
                {{ dealSide(item.side).name }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'insert_time'">
              {{ dealKfTime(item.insert_time) }}
            </template>
            <template v-else-if="column.dataIndex === 'price_level'">
              {{ dealPriceLevel(item.price_level).name }}
            </template>
            <template v-else-if="column.dataIndex === 'progress'">
              <a-progress :percent="item.progress" size="small" />
            </template>
            <template v-else-if="column.dataIndex === 'status_uname'">
              <span :class="`color-${item.status_color}`">
                {{ item.status_uname }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'actions'">
              <div class="kf-actions__warp">
                <a-button
                  type="link"
                  size="small"
                  class="color-red"
                  @click.stop="handleCancelBasketOrder(item)"
                >
                  {{ $t('BasketTrade.cancel_order') }}
                </a-button>
              </div>
            </template>
          </template>
        </KfTradingDataTable>
      </div>
    </KfDashboard>
    <!-- <StatisticModal
      v-if="statisticModalVisible"
      v-model:visible="statisticModalVisible"
      :orders="tableData"
      :is-unfinished-order="unfinishedOrder"
      :history-date="historyDate"
    ></StatisticModal> -->
  </div>
</template>

<script lang="ts" setup>
import {
  ref,
  onMounted,
  watch,
  getCurrentInstance,
  onBeforeUnmount,
} from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
// import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';

import {
  dealPriceLevel,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  buildBasketOrderMapKeyAndLocation,
  dealBasketOrdersToMap,
  useCurrentGlobalBasket,
  useMakeOrCancelBasketOrder,
} from '../../../utils/basketTradeUtils';
import {
  useDashboardBodySize,
  confirmModal,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { BASKET_CATEGORYS } from '../../../config';
import { getColumns, basketOrderTradingDataGetter } from './config';
import { DealTradingDataHooks } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { useBasketTradeStore } from '../../../store';
const { t } = VueI18n.global;

(
  globalThis.HookKeeper.getHooks().dealTradingData as DealTradingDataHooks
).register(
  {
    category: BASKET_CATEGORYS.ORDER,
    group: '*',
    name: '*',
  } as KungfuApi.KfExtraLocation,
  basketOrderTradingDataGetter,
);

const app = getCurrentInstance();
const { updateBasketOrdersMap } = useBasketTradeStore();
const {
  currentGlobalBasket,
  currentGlobalBasketOrder,
  currentBasketData,
  dealBasketOrderRowClassName,
  setCurrentGlobalBasketOrder,
} = useCurrentGlobalBasket();
const { handleBodySizeChange } = useDashboardBodySize();
const { cancalBasketOrder } = useMakeOrCancelBasketOrder();

const basketOrdersMap = ref<Record<string, KungfuApi.BasketOrderResolved>>({});
const basketOrders = ref<KungfuApi.BasketOrderResolved[]>([]);

const hasMakeNewBasketOrder = ref(false);

const columns = getColumns();

const setDefaultGlobalBasketOrder = () => {
  if (!currentGlobalBasketOrder.value) {
    if (basketOrders.value.length) {
      setCurrentGlobalBasketOrder(basketOrders.value[0]);
    }
  } else {
    if (hasMakeNewBasketOrder.value) {
      setCurrentGlobalBasketOrder(
        basketOrders.value.length ? basketOrders.value[0] : null,
      );
      hasMakeNewBasketOrder.value = false;
    }

    const curBasketOrderKey = buildBasketOrderMapKeyAndLocation(
      currentGlobalBasketOrder.value,
    ).key;

    if (!(curBasketOrderKey in basketOrdersMap.value)) {
      setCurrentGlobalBasketOrder(
        basketOrders.value.length ? basketOrders.value[0] : null,
      );
    }
  }
};

const getBasketOrders = (watcher: KungfuApi.Watcher) => {
  if (!currentGlobalBasket.value) {
    basketOrdersMap.value = {};
    basketOrders.value = [];
    return;
  }

  const basketOrderListFiltered = watcher.ledger.BasketOrder.filter(
    'parent_id',
    BigInt(currentGlobalBasket.value.id),
  ).sort('insert_time');

  basketOrdersMap.value = dealBasketOrdersToMap(basketOrderListFiltered);

  basketOrders.value = Object.values(basketOrdersMap.value);

  setDefaultGlobalBasketOrder();
};

onMounted(() => {
  const tradingDataSubscribtion = app?.proxy?.$tradingDataSubject.subscribe(
    (watcher: KungfuApi.Watcher) => {
      const basketOrderList = watcher.ledger.BasketOrder.sort('insert_time');
      updateBasketOrdersMap(dealBasketOrdersToMap(basketOrderList));

      getBasketOrders(watcher);
    },
  );

  const globalBusSubscribtion = app?.proxy?.$globalBus.subscribe((data) => {
    if (data.tag === 'update:makeBasketOrder') {
      hasMakeNewBasketOrder.value = true;
    }
  });

  onBeforeUnmount(() => {
    tradingDataSubscribtion?.unsubscribe();
    globalBusSubscribtion?.unsubscribe();
  });
});

watch(
  () => currentGlobalBasket.value,
  (newBasket) => {
    if (!newBasket) return;

    getBasketOrders(window.watcher);
  },
);

function handleClickRow(data: {
  event: MouseEvent;
  row: KungfuApi.BasketOrderResolved;
  column: KfTradingDataTableHeaderConfig;
}) {
  setCurrentGlobalBasketOrder(data.row);
}

function dealRowClassNameResolved(record: KungfuApi.BasketOrderResolved) {
  return dealBasketOrderRowClassName(record);
}

function handleCancelBasketOrder(basketOrder: KungfuApi.BasketOrderResolved) {
  if (!currentGlobalBasket.value || !currentGlobalBasketOrder.value)
    return Promise.reject();

  return confirmModal(
    t('orderConfig.confirm_cancel_all'),
    `${t('orderConfig.confirm')} ${t('BasketTrade.basket_order')} ${dealKfTime(
      currentGlobalBasketOrder.value.insert_time,
    )} ${t('orderConfig.cancel_all')}`,
  ).then((flag) => {
    if (!flag || !currentGlobalBasketOrder.value || !window.watcher) {
      return;
    }

    return cancalBasketOrder(window.watcher, basketOrder);
  });
}
</script>

<style lang="less"></style>
