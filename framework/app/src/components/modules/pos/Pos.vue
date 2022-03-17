<script setup lang="ts">
import {
  dealKfPrice,
  dealAssetPrice,
  dealDirection,
  dealTradingData,
  findTargetFromArray,
  isTdStrategyCategory,
  getIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useCurrentGlobalKfLocation,
  useDownloadHistoryTradingData,
  useTableSearchKeyword,
  useTriggerMakeOrder,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import { DownloadOutlined } from '@ant-design/icons-vue';

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
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import {
  dealPosition,
  hashInstrumentUKey,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import {
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  getInstrumentByInstrumentPair,
  useInstruments,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { useExtraCategory } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

const app = getCurrentInstance();
const pos = ref<KungfuApi.Position[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.Position>(
  pos,
  [
    'instrument_id',
    'exchange_id',
    'direction',
    'source_id',
    'account_id',
    'client_id',
  ],
);
const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);
const { handleDownload } = useDownloadHistoryTradingData();
const { triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();
const { instruments } = useInstruments();
const { getExtraCategoryData } = useExtraCategory();

const columns = computed(() => {
  if (currentGlobalKfLocation.data === null) {
    return getColumns('td');
  }

  const { category } = currentGlobalKfLocation.data;
  return getColumns(category);
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        console.log("Pos.vue 111");
        if (currentGlobalKfLocation.data === null) {
        console.log("Pos.vue 222");
          return;
        }

        const positions = isTdStrategyCategory(
          currentGlobalKfLocation.data.category,
        )
          ? ((dealTradingData(
              watcher,
              watcher.ledger,
              'Position',
              currentGlobalKfLocation.data,
            ) || []) as KungfuApi.Position[])
          : (getExtraCategoryData(
              watcher.ledger.Position,
              currentGlobalKfLocation.data,
              'position',
            ) as KungfuApi.Position[]);

        pos.value = toRaw(
          positions.reverse().map((item) => dealPosition(watcher, item)),
        );
        console.log("Pos.vue 333");
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(currentGlobalKfLocation, () => {
  pos.value = [];
});

function handleClickRow(data: {
  event: MouseEvent;
  row: TradingDataItem;
  column: KfTradingDataTableHeaderConfig;
}) {
  const row = data.row as KungfuApi.Position;
  const { instrument_id, instrument_type, exchange_id } = row;
  const ensuredInstrument: KungfuApi.InstrumentResolved =
    getInstrumentByInstrumentPair(
      {
        instrument_id,
        instrument_type,
        exchange_id,
      },
      instruments.data,
    );

  triggerOrderBook(ensuredInstrument);
  const extraOrderInput: ExtraOrderInput = {
    side: row.direction === 0 ? SideEnum.Sell : SideEnum.Buy,
    offset:
      row.yesterday_volume !== BigInt(0)
        ? OffsetEnum.CloseYest
        : OffsetEnum.CloseToday,
    volume:
      row.yesterday_volume !== BigInt(0)
        ? row.yesterday_volume
        : row.volume - row.yesterday_volume,

    price: row.last_price || 0,
    accountId: isTdStrategyCategory(currentGlobalKfLocation.data?.category)
      ? undefined
      : dealLocationUIDResolved(row.holder_uid),
  };
  triggerMakeOrder(ensuredInstrument, extraOrderInput);
}

function dealLocationUIDResolved(holderUID: number): string {
  console.log("9...")
  return getIdByKfLocation(window.watcher.getLocation(holderUID));
}
</script>
<template>
  <div class="kf-position__warp kf-translateZ">
    <KfDashboard>
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation.data">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation.data">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation.data) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            placeholder="关键字"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            @click="handleDownload('Position', currentGlobalKfLocation.data)"
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
        keyField="uid_key"
        @clickCell="handleClickRow"
      >
        <template
          v-slot:default="{
            item,
            column,
          }: {
            item: KungfuApi.Position,
            column: KfTradingDataTableHeaderConfig,
          }"
        >
          <template v-if="column.dataIndex === 'direction'">
            <span :class="`color-${dealDirection(item.direction).color}`">
              {{ dealDirection(item.direction).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'yesterday_volume'">
            <KfBlinkNum
              :num="Number(item.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'today_volume'">
            <KfBlinkNum
              :num="Number(item.volume - item.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'volume'">
            <KfBlinkNum :num="Number(item.volume).toFixed(0)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'avg_open_price'">
            <KfBlinkNum :num="dealKfPrice(item.avg_open_price)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'last_price'">
            <KfBlinkNum :num="dealKfPrice(item.last_price)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'unrealized_pnl'">
            <KfBlinkNum
              mode="compare-zero"
              :num="dealAssetPrice(item.unrealized_pnl)"
            ></KfBlinkNum>
          </template>
        </template>
      </KfTradingDataTable>
    </KfDashboard>
  </div>
</template>
<style lang="less">
.kf-position__warp {
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
