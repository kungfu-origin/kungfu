<script setup lang="ts">
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
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
    historyDate: null,
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

const priceVolumeStats = computed(() => {
  const tradesForStatistic = props.trades
    .slice(0)
    .filter((item) => item.price >= 0);
  const priceVolumeData: Record<
    string,
    {
      price: number[];
      volume: number[];
    }
  > = tradesForStatistic.reduce(
    (
      priceVolumeData: Record<
        string,
        {
          price: number[];
          volume: number[];
        }
      >,
      trade: KungfuApi.TradeResolved,
    ) => {
      const id = `${trade.instrument_id}_${trade.exchange_id}_${trade.side}_${trade.offset}`;
      if (!priceVolumeData[id]) {
        priceVolumeData[id] = {
          price: [],
          volume: [],
        };
      }

      priceVolumeData[id].price.push(trade.price);
      priceVolumeData[id].volume.push(+Number(trade.volume));
      return priceVolumeData;
    },
    {} as Record<
      string,
      {
        price: number[];
        volume: number[];
        volumeTraded: number[];
      }
    >,
  );

  const priceVolumeDataResolved: Array<{
    instrumentId_exchangeId: string;
    side: KungfuApi.KfTradeValueCommonData;
    offset: KungfuApi.KfTradeValueCommonData;
    mean: string;
    min: string;
    max: string;
    volume: string;
  }> = Object.keys(priceVolumeData).map((id) => {
    const [instrumentId, exchangeId, side, offset] = id.split('_');
    const priceStats = new Stats().push(...priceVolumeData[id].price);
    const volumeSum = priceVolumeData[id].volume.reduce((a, b) => a + b);
    const range = priceStats.range();
    return {
      instrumentId_exchangeId: `${instrumentId}_${exchangeId}`,
      side: dealSide(+side),
      offset: dealOffset(+offset),
      mean: priceStats.amean().toFixed(2),
      min: range[0].toFixed(2),
      max: range[1].toFixed(2),
      volume: volumeSum.toString(),
    };
  });

  return priceVolumeDataResolved;
});
</script>
<template>
  <a-modal
    :width="720"
    class="kf-trade-statistic-modal"
    v-model:visible="modalVisible"
    :title="`成交统计 ${
      !!historyDate
        ? historyDate.format('YYYY-MM-DD HH:mm:ss')
        : '实时(最新100条数据)'
    }`"
    :destroyOnClose="true"
    :footer="null"
    @cancel="closeModal"
  >
    <a-row style="margin-bottom: 30px">
      <a-col>
        <a-statistic title="统计成交数量" :value="trades.length"></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px" class="limit-price-stats-row">
      <div class="title">成交价统计</div>
      <a-table
        v-if="priceVolumeStats"
        size="small"
        :dataSource="priceVolumeStats"
        :columns="statisColums"
        :pagination="false"
      >
        <template #bodyCell="{ column, record }">
          <template v-if="column.dataIndex === 'side'">
            <span :class="`color-${record.side.color}`">
              {{ record.side.name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'offset'">
            <span :class="`color-${record.offset.color}`">
              {{ record.offset.name }}
            </span>
          </template>
        </template>
      </a-table>
    </a-row>
    <a-row style="margin-bottom: 30px">
      <a-col :span="8">
        <a-statistic
          title="平均成交延迟(μs)"
          :value="tradeLatencyStats.mean"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          title="最小成交延迟(μs)"
          :value="tradeLatencyStats.min"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          title="最大成交延迟(μs)"
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
    }
  }
}
</style>
