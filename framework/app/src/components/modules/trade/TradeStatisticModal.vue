<script setup lang="ts">
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import {
  useModalVisible,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import { Stats } from 'fast-stats';
import {
  dealOffset,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { statisColums } from './config';
import { Dayjs } from 'dayjs';

const props = withDefaults(
  defineProps<{
    visible: boolean;
    trades: KungfuApi.TradeResolved[];
    historyDate?: Dayjs;
  }>(),
  {
    visible: false,
    trades: () => [],
    historyDate: undefined,
  },
);

defineEmits<{
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);

const tradeLatencyStats = computed(() => {
  const tradesForStatistic = props.trades.slice(0);
  const tradeLatencyBuckets = tradesForStatistic
    .filter((item) => +item.latency_trade > 0 && item.latency_trade !== '--')
    .map((item) => +item.latency_trade);

  if (!tradeLatencyBuckets.length) {
    return {
      mean: '--',
      min: '--',
      max: '--',
    };
  }

  const stats = new Stats().push(...tradeLatencyBuckets);
  const range = stats.range();
  return {
    mean: stats.amean().toFixed(2),
    min: range[0],
    max: range[1],
  };
});

interface PriceVolumeStatItem {
  price: number[];
  volume: number[];
  priceByVolume: number[];
}

const priceVolumeStats = computed(() => {
  const tradesForStatistic = props.trades
    .slice(0)
    .filter((item) => item.price >= 0);
  const priceVolumeData: Record<string, PriceVolumeStatItem> =
    tradesForStatistic.reduce(
      (
        priceVolumeData: Record<string, PriceVolumeStatItem>,
        trade: KungfuApi.TradeResolved,
      ) => {
        const id = `${trade.instrument_id}_${trade.exchange_id}_${trade.side}_${trade.offset}`;
        if (!priceVolumeData[id]) {
          priceVolumeData[id] = {
            price: [],
            volume: [],
            priceByVolume: [],
          };
        }

        priceVolumeData[id].price.push(trade.price);
        priceVolumeData[id].volume.push(+Number(trade.volume));
        priceVolumeData[id].priceByVolume.push(
          +Number(trade.volume) * trade.price,
        );
        return priceVolumeData;
      },
      {} as Record<string, PriceVolumeStatItem>,
    );

  const priceVolumeDataResolved: Array<{
    instrumentId_exchangeId: string;
    sideName: string;
    sideColor: KungfuApi.AntInKungfuColorTypes;
    offsetName: string;
    offsetColor: KungfuApi.AntInKungfuColorTypes;
    mean: string;
    min: string;
    max: string;
    volume: string;
  }> = Object.keys(priceVolumeData)
    .map((id) => {
      const [instrumentId, exchangeId, side, offset] = id.split('_');
      const priceStats = new Stats().push(...priceVolumeData[id].price);
      const priceSum = priceVolumeData[id].priceByVolume.reduce(
        (a, b) => a + b,
      );
      const volumeSum = priceVolumeData[id].volume.reduce((a, b) => a + b);
      const range = priceStats.range();
      const sideReolved = dealSide(+side);
      const offsetResolved = dealOffset(+offset);
      return {
        id,
        instrumentId_exchangeId: `${instrumentId}_${exchangeId}`,
        sideName: sideReolved.name,
        sideColor: sideReolved.color || 'default',
        offsetName: offsetResolved.name,
        offsetColor: offsetResolved.color || 'default',
        mean: Number(priceSum / volumeSum).toFixed(2),
        min: range[0].toFixed(2),
        max: range[1].toFixed(2),
        volume: volumeSum.toString(),
      };
    })
    .sort((a, b) =>
      a.instrumentId_exchangeId.localeCompare(b.instrumentId_exchangeId),
    );

  return priceVolumeDataResolved;
});

const { searchKeyword, tableData } = useTableSearchKeyword(priceVolumeStats, [
  'instrumentId_exchangeId',
  'side',
  'offset',
  'mean',
  'min',
  'max',
  'volume',
]);
</script>
<template>
  <a-modal
    :width="720"
    class="kf-trade-statistic-modal"
    v-model:visible="modalVisible"
    :title="`${$t('tradeConfig.statistical')} ${
      !!historyDate
        ? historyDate.format('YYYY-MM-DD HH:mm:ss')
        : $t('tradeConfig.statistical_desc')
    }`"
    :destroyOnClose="true"
    :footer="null"
    @cancel="closeModal"
  >
    <a-row style="margin-bottom: 30px">
      <a-col>
        <a-statistic
          :title="$t('tradeConfig.statistical_count')"
          :value="trades.length"
        ></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px" class="limit-price-stats-row">
      <div class="title">
        <span>{{ $t('tradeConfig.statistical_price') }}</span>
        <a-input-search
          v-model:value="searchKeyword"
          :placeholder="$t('keyword_input')"
          style="width: 120px"
          size="small"
        />
      </div>
      <div class="table" style="height: 260px">
        <KfTradingDataTable
          v-if="priceVolumeStats"
          key-field="id"
          :data-source="tableData"
          :columns="statisColums"
        >
          <template #default="{ column, item }">
            <template v-if="column.dataIndex === 'sideName'">
              <span :class="`color-${item.sideColor}`">
                {{ item.sideName }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'offsetName'">
              <span :class="`color-${item.offsetColor}`">
                {{ item.offsetName }}
              </span>
            </template>
          </template>
        </KfTradingDataTable>
      </div>
    </a-row>
    <a-row style="margin-bottom: 30px">
      <a-col :span="8">
        <a-statistic
          :title="$t('tradeConfig.average_trade_latency')"
          :value="tradeLatencyStats.mean"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('tradeConfig.min_trade_latency')"
          :value="tradeLatencyStats.min"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('tradeConfig.max_trade_latency')"
          :value="tradeLatencyStats.max"
        ></a-statistic>
      </a-col>
    </a-row>
  </a-modal>
</template>
<style lang="less">
.kf-trade-statistic-modal {
  .limit-price-stats-row {
    flex-direction: column;
    .title {
      font-size: 12px;
      color: rgba(255, 255, 255, 0.45);
      margin-bottom: 8px;

      display: flex;
      justify-content: space-between;
    }
  }
}
</style>
