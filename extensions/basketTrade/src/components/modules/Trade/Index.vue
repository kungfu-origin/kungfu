<script setup lang="ts">
import {
  dealKfPrice,
  dealSide,
  dealOffset,
  delayMilliSeconds,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  messagePrompt,
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
import TradeStatisticModal from './TradeStatisticModal.vue';
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useCurrentGlobalBasket } from '../../../utils/basketTradeUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;
const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();
const { currentGlobalBasket, currentBasketData } = useCurrentGlobalBasket();

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

const { currentGlobalKfLocation } = useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();
const statisticModalVisible = ref<boolean>(false);

const columns = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return getColumns(
      {
        category: 'td',
        group: '*',
        name: '*',
        mode: 'live',
      },
      !!historyDate.value,
    );
  }

  return getColumns(currentGlobalKfLocation.value, !!historyDate.value);
});

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

        const tradesResolved =
          globalThis.HookKeeper.getHooks().dealTradingData.trigger(
            watcher,
            currentGlobalKfLocation.value,
            watcher.ledger.Trade,
            'trade',
          ) as KungfuApi.Trade[];

        trades.value = toRaw(
          tradesResolved
            .slice(0, 500)
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
  delayMilliSeconds(500)
    .then(() =>
      getKungfuHistoryData(
        newDate.format(),
        HistoryDateEnum.naturalDate,
        'Trade',
        currentGlobalKfLocation.value as KungfuApi.KfLocation,
      ),
    )
    .then((historyData) => {
      if (!historyData) return;

      const { tradingData } = historyData;

      const tradesResolved =
        globalThis.HookKeeper.getHooks().dealTradingData.trigger(
          window.watcher,
          currentGlobalKfLocation.value,
          tradingData.Trade,
          'trade',
        ) as KungfuApi.Trade[];

      trades.value = toRaw(
        tradesResolved.map((item) =>
          toRaw(dealTrade(window.watcher, item, tradingData.OrderStat, true)),
        ),
      );
      historyDataLoading.value = false;
    })
    .catch((err) => {
      if (err.message === 'database_locked') {
        messagePrompt().error(t('database_locked'));
      } else {
        console.error(err.message);
      }
    });
});

function handleShowTradingDataDetail({
  row,
}: {
  event: MouseEvent;
  row: KungfuApi.TradingDataItem;
}) {
  showTradingDataDetail(row as KungfuApi.TradeResolved, '成交');
}
</script>
<template>
  <div class="kf-trades__warp kf-translateZ">
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
        :data-source="tableData"
        key-field="trade_id"
        @right-click-row="handleShowTradingDataDetail"
      >
        <template
          #default="{
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
      :history-date="historyDate"
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
