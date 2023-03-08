<script setup lang="ts">
import {
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
import { storeToRefs } from 'pinia';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import { categoryRegisterConfig, getColumns } from './config';
import {
  dealAssetPrice,
  dealDirection,
  dealKfPrice,
  dealCurrency,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  LedgerCategoryEnum,
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import { ExchangeIds } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  getInstrumentByInstrumentPair,
  useCurrentGlobalKfLocation,
  useInstruments,
  useActiveInstruments,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import {
  dealPosition,
  getPosClosableVolume,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';

globalThis.HookKeeper.getHooks().dealTradingData.register(
  {
    category: categoryRegisterConfig.category,
    group: '*',
    name: '*',
  },
  categoryRegisterConfig,
);

const app = getCurrentInstance();
const columns = getColumns();
const pos = ref<KungfuApi.PositionResolved[]>([]);
const { handleBodySizeChange } = useDashboardBodySize();
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.PositionResolved>(pos, [
    'instrument_id_resolved',
    'instrument_id',
    'exchange_id',
    'direction',
  ]);

const {
  currentGlobalKfLocation,
  dealRowClassName,
  setCurrentGlobalKfLocation,
} = useCurrentGlobalKfLocation(window.watcher);
const { instruments } = useInstruments();
const { triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();
const { getInstrumentCurrencyByIds } = useActiveInstruments();
const { globalSetting } = storeToRefs(useGlobalStore());

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        setTimeout(() => {
          const positions = watcher.ledger.Position.nofilter(
            'volume',
            BigInt(0),
          )
            .filter('ledger_category', LedgerCategoryEnum.td)
            .list();

          pos.value = toRaw(
            buildGlobalPositions(positions).map((position) =>
              dealPosition(window.watcher, position),
            ),
          );
        });
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
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
        uid_key: pos.uid_key,
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

function dealRowClassNameResolved(row: KungfuApi.PositionResolved) {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: categoryRegisterConfig.category,
    group: row.exchange_id,
    name: row.instrument_id,
    mode: 'live',
    direction: row.direction,
  };

  const classNameResolved = dealRowClassName(locationResolved);

  if (
    classNameResolved &&
    (currentGlobalKfLocation.value as KungfuApi.KfExtraLocation)?.direction ===
      row.direction
  ) {
    return classNameResolved;
  }

  return '';
}

function handleClickRow(data: {
  event: MouseEvent;
  row: KungfuApi.PositionResolved;
  column: KfTradingDataTableHeaderConfig;
}) {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: categoryRegisterConfig.category,
    group: data.row.exchange_id,
    name: data.row.instrument_id,
    mode: 'live',
    direction: data.row.direction,
  };

  setCurrentGlobalKfLocation(locationResolved);
  tiggerOrderBookAndMakeOrder(data.row);
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
      instruments.value,
    );

  triggerOrderBook(ensuredInstrument);
  const extraOrderInput: ExtraOrderInput = {
    side: record.direction === 0 ? SideEnum.Sell : SideEnum.Buy,
    offset:
      record.yesterday_volume !== BigInt(0)
        ? OffsetEnum.CloseYest
        : OffsetEnum.CloseToday,
    volume: getPosClosableVolume(record),

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
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
      </template>
      <KfTradingDataTable
        class="kf-ant-table"
        key-field="uid_key"
        :columns="columns"
        :data-source="tableData"
        :item-size="28"
        :custom-row-class="dealRowClassNameResolved"
        @clickCell="handleClickRow"
      >
        <template
          v-slot:default="{
            column,
            item,
          }: {
            column: KfTradingDataTableHeaderConfig,
            item: KungfuApi.PositionResolved,
          }"
        >
          <template v-if="column.dataIndex === 'instrument_id'">
            <span>
              {{ item.instrument_id }}
              {{ ExchangeIds[item.exchange_id].name }}
              <span
                v-if="globalSetting.value?.currency?.instrumentCurrency"
                style="margin-left: 4px; color: #faad14"
              >
                {{
                  dealCurrency(
                    getInstrumentCurrencyByIds(
                      item.instrument_id,
                      item.exchange_id,
                    ),
                  ).name
                }}
              </span>
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'direction'">
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
.kf-position-global__warp {
  height: 100%;
}
</style>
