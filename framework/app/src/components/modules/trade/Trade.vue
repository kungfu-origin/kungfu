<script setup lang="ts">
import {
  dealKfPrice,
  dealSide,
  dealOffset,
  delayMilliSeconds,
  dealTradingData,
  isTdStrategyCategory,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useDashboardBodySize,
  useDownloadHistoryTradingData,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import {
  DownloadOutlined,
  LoadingOutlined,
  CalendarOutlined,
  PieChartOutlined,
} from '@ant-design/icons-vue';

import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
  toRaw,
  watch,
} from 'vue';
import { getColumns } from './config';
import {
  dealTrade,
  getKungfuHistoryData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import {
  showTradingDataDetail,
  useCurrentGlobalKfLocation,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';
import TradeStatisticModal from './TradeStatisticModal.vue';
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { getKfGlobalSettingsValue } from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import path from 'path';
import { KUNGFU_RESOURCES_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';

const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();

const trades = ref<KungfuApi.TradeResolved[]>([]);
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.TradeResolved>(trades, [
    'order_id',
    'trade_id',
    'instrument_id',
    'exchange_id',
    'source_uname',
    'dest_uname',
  ]);
const historyDate = ref<Dayjs>();
const historyDataLoading = ref<boolean>();

const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();
const { getExtraCategoryData } = useExtraCategory();
const statisticModalVisible = ref<boolean>(false);

const columns = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return getColumns('td', !!historyDate.value);
  }

  const category = currentGlobalKfLocation.value?.category;
  return getColumns(category, !!historyDate.value);
});

const lastTradeId: {
  value: bigint;
} = {
  value: 0n,
};

const isPlaySound = getKfGlobalSettingsValue()?.trade?.sound || false;
const soundPath = path.join(
  `${path.join(KUNGFU_RESOURCES_DIR, 'music/ding.mp3')}`,
);

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (historyDate.value) {
          return;
        }

        if (currentGlobalKfLocation.value === null) {
          return;
        }

        const tradesResolved = isTdStrategyCategory(
          currentGlobalKfLocation.value?.category,
        )
          ? ((dealTradingData(
              watcher,
              watcher.ledger,
              'Trade',
              currentGlobalKfLocation.value,
            ) || []) as KungfuApi.Trade[])
          : (getExtraCategoryData(
              watcher.ledger.Trade,
              currentGlobalKfLocation.value,
              'trade',
            ) as KungfuApi.Trade[]);

        trades.value = toRaw(
          tradesResolved
            .slice(0, 100)
            .map((item) =>
              toRaw(dealTrade(watcher, item, watcher.ledger.OrderStat)),
            ),
        );
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(currentGlobalKfLocation, () => {
  historyDate.value = undefined;
  trades.value = [];
});

watch(historyDate, async (newDate) => {
  if (!newDate) {
    return;
  }

  if (currentGlobalKfLocation.value === null) {
    return;
  }

  trades.value = [];
  historyDataLoading.value = true;
  await delayMilliSeconds(500);
  const { tradingData, historyDatas } = await getKungfuHistoryData(
    window.watcher,
    newDate.format(),
    HistoryDateEnum.naturalDate,
    'Trade',
    currentGlobalKfLocation.value,
  );

  const tradesResolved = isTdStrategyCategory(
    currentGlobalKfLocation.value?.category,
  )
    ? toRaw(historyDatas as KungfuApi.Trade[])
    : (getExtraCategoryData(
        tradingData.Trade,
        currentGlobalKfLocation.value,
        'trade',
      ) as KungfuApi.Trade[]);

  trades.value = toRaw(
    tradesResolved.map((item) =>
      toRaw(dealTrade(window.watcher, item, tradingData.OrderStat, true)),
    ),
  );
  historyDataLoading.value = false;
});

function handleShowTradingDataDetail({
  row,
}: {
  event: MouseEvent;
  row: TradingDataItem;
}) {
  showTradingDataDetail(row as KungfuApi.TradeResolved, '成交');
}
</script>
<template>
  <div class="kf-trades__warp kf-translateZ">
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
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-date-picker
            v-model:value="historyDate"
            :disabled="historyDataLoading"
          >
            <template #suffixIcon>
              <LoadingOutlined v-if="historyDataLoading" />
              <CalendarOutlined v-else />
            </template>
          </a-date-picker>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="statisticModalVisible = true">
            <template #icon>
              <PieChartOutlined style="font-size: 14px"></PieChartOutlined>
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            @click="handleDownload('Trade', currentGlobalKfLocation)"
          >
            <template #icon>
              <DownloadOutlined style="font-size: 14px" />
            </template>
          </a-button>
        </KfDashboardItem>
      </template>
      <KfTradingDataTable
        :columns="columns"
        :dataSource="tableData"
        keyField="trade_id"
        @rightClickRow="handleShowTradingDataDetail"
      >
        <template
          v-slot:default="{
            item,
            column,
          }: {
            item: KungfuApi.TradeResolved,
            column: KfTradingDataTableHeaderConfig,
          }"
        >
          <template v-if="column.dataIndex === 'side'">
            <span :class="`color-${dealSide(item.side).color}`">
              {{ dealSide(item.side).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'offset'">
            <span :class="`color-${dealOffset(item.offset).color}`">
              {{ dealOffset(item.offset).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'price'">
            {{ dealKfPrice(item.price) }}
          </template>
          <template v-else-if="column.dataIndex === 'source_uname'">
            <span :class="[`color-${item.source_resolved_data.color}`]">
              {{ item.source_uname }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'dest_uname'">
            <span :class="[`color-${item.dest_resolved_data.color}`]">
              {{ item.dest_uname }}
            </span>
          </template>
        </template>
      </KfTradingDataTable>
    </KfDashboard>
    <TradeStatisticModal
      v-if="statisticModalVisible"
      v-model:visible="statisticModalVisible"
      :trades="tableData"
      :historyDate="historyDate"
    ></TradeStatisticModal>
  </div>
</template>
<style lang="less">
.kf-trades__warp {
  width: 100%;
  height: 100%;

  .kf-table__warp {
    width: 100%;
    height: 100%;

    .kf-trading-data-table {
      width: 100%;
      height: 100%;
    }
  }
}
</style>
