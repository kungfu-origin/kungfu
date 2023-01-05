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
          :data-source="Object.values(basketOrdersMap)"
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
            <template v-else-if="column.dataIndex === 'price_resolved'">
              {{ dealKfPrice(item.price_resolved) }}
            </template>
            <template v-else-if="column.dataIndex === 'progress'">
              <a-progress :precent="item.progress" size="small" />
            </template>
            <template v-else-if="column.dataIndex === 'status_uname'">
              <span :class="`color-${item.status_color}`">
                {{ item.status_uname }}
              </span>
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
import { ref, onMounted, getCurrentInstance } from 'vue';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
// import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';

import {
  dealKfPrice,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  dealBasketOrdersToMap,
  useCurrentGlobalBasket,
} from '../../../utils/basketTradeUtils';
import { useDashboardBodySize } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useCurrentGlobalKfLocation } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { getColumns } from './config';

const app = getCurrentInstance();

const { dealRowClassName, customRow } = useCurrentGlobalKfLocation(
  window.watcher,
);
const { currentGlobalBasket, currentBasketData } = useCurrentGlobalBasket();
const { handleBodySizeChange } = useDashboardBodySize();

const basketOrdersMap = ref<Record<string, KungfuApi.BasketOrderResolved>>({});

const columns = getColumns();

onMounted(() => {
  app?.proxy?.$tradingDataSubject.subscribe((watcher: KungfuApi.Watcher) => {
    basketOrdersMap.value = dealBasketOrdersToMap(
      watcher.ledger.BasketOrder.list(),
    );
  });
});

function handleClickRow(data: {
  event: MouseEvent;
  row: KungfuApi.BasketOrderResolved;
  column: KfTradingDataTableHeaderConfig;
}) {
  customRow(data.row.basket_order_location).onClick();
}

function dealRowClassNameResolved(record: KungfuApi.BasketOrderResolved) {
  return dealRowClassName(record.basket_order_location);
}
</script>

<style lang="less"></style>
