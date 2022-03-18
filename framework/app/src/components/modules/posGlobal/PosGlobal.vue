<script setup lang="ts">
import {
  useCurrentGlobalKfLocation,
  useDashboardBodySize,
  useTableSearchKeyword,
  useTriggerMakeOrder,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
  toRaw,
} from 'vue';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import { categoryRegisterConfig, columns } from './config';
import {
  dealAssetPrice,
  dealDirection,
  dealKfPrice,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  LedgerCategoryEnum,
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  getInstrumentByInstrumentPair,
  useInstruments,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';

const app = getCurrentInstance();
const pos = ref<KungfuApi.Position[]>([]);
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.Position>(
  pos,
  ['instrument_id', 'exchange_id', 'direction'],
);

const { dealRowClassName, setCurrentGlobalKfLocation } =
  useCurrentGlobalKfLocation(window.watcher);
const { instruments } = useInstruments();
const { triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        const positions = watcher.ledger.Position.nofilter('volume', BigInt(0))
          .filter('ledger_category', LedgerCategoryEnum.td)
          .list();
        pos.value = toRaw(buildGlobalPositions(positions));
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });

    app.proxy.$globalCategoryRegister.register(categoryRegisterConfig);
  }
});

type PosStat = Record<string, KungfuApi.Position>;

function buildGlobalPositions(
  positions: KungfuApi.Position[],
): KungfuApi.Position[] {
  const posStatData: PosStat = positions.reduce((posStat, pos) => {
    const id = `${pos.exchange_id}_${pos.instrument_id}_${pos.direction}`;
    if (!posStat[id]) {
      posStat[id] = pos;
    } else {
      const prePosStat = posStat[id];
      const { avg_open_price, volume, yesterday_volume, unrealized_pnl } =
        prePosStat;
      posStat[id] = {
        ...prePosStat,
        yesterday_volume: yesterday_volume + pos.yesterday_volume,
        volume: volume + pos.volume,

        avg_open_price:
          (avg_open_price * Number(volume) +
            pos.avg_open_price * Number(pos.volume)) /
          (Number(pos.volume) + Number(pos.volume)),
        unrealized_pnl: unrealized_pnl + pos.unrealized_pnl,
      };
    }
    return posStat;
  }, {} as PosStat);

  return Object.values(posStatData).sort((item1, item2) => {
    const id1 = `${item1.instrument_id}_${item1.instrument_id}`;
    const id2 = `${item2.instrument_id}_${item2.instrument_id}`;
    return id1.localeCompare(id2);
  });
}

function dealRowClassNameResolved(record: KungfuApi.Position) {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: categoryRegisterConfig.name,
    group: record.exchange_id,
    name: record.instrument_id,
    mode: 'LIVE',
  };

  return dealRowClassName(locationResolved);
}

function customRowResolved(record: KungfuApi.Position) {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: categoryRegisterConfig.name,
    group: record.exchange_id,
    name: record.instrument_id,
    mode: 'LIVE',
  };

  return {
    onClick: () => {
      setCurrentGlobalKfLocation(locationResolved);
      tiggerOrderBookAndMakeOrder(record);
    },
  };
}

function tiggerOrderBookAndMakeOrder(record: KungfuApi.Position) {
  const { instrument_id, instrument_type, exchange_id } = record;
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
    side: record.direction === 0 ? SideEnum.Sell : SideEnum.Buy,
    offset:
      record.yesterday_volume !== BigInt(0)
        ? OffsetEnum.CloseYest
        : OffsetEnum.CloseToday,
    volume:
      record.yesterday_volume !== BigInt(0)
        ? record.yesterday_volume
        : record.volume - record.yesterday_volume,

    price: record.last_price || 0,
  };
  triggerMakeOrder(ensuredInstrument, extraOrderInput);
}
</script>
<template>
  <div class="kf-position-global__warp kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            placeholder="关键字"
            style="width: 120px"
          />
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableData"
        size="small"
        :pagination="false"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :rowClassName="dealRowClassNameResolved"
        :customRow="customRowResolved"
        emptyText="暂无数据"
      >
        <template
          #bodyCell="{
            column,
            record,
          }: {
            column: AntTableColumn,
            record: KungfuApi.Position,
          }"
        >
          <template v-if="column.dataIndex === 'instrument_id'">
            {{ record.instrument_id }}
            {{ ExchangeIds[record.exchange_id].name }}
          </template>
          <template v-else-if="column.dataIndex === 'direction'">
            <span :class="`color-${dealDirection(record.direction).color}`">
              {{ dealDirection(record.direction).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'yesterday_volume'">
            <KfBlinkNum
              :num="Number(record.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'today_volume'">
            <KfBlinkNum
              :num="Number(record.volume - record.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'volume'">
            <KfBlinkNum :num="Number(record.volume).toFixed(0)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'avg_open_price'">
            <KfBlinkNum :num="dealKfPrice(record.avg_open_price)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'last_price'">
            <KfBlinkNum :num="dealKfPrice(record.last_price)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'unrealized_pnl'">
            <KfBlinkNum
              mode="compare-zero"
              :num="dealAssetPrice(record.unrealized_pnl)"
            ></KfBlinkNum>
          </template>
        </template>
      </a-table>
    </KfDashboard>
  </div>
</template>
<style lang="less">
.kf-position-global__warp {
  height: 100%;
}
</style>
