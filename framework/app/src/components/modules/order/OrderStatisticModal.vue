<script setup lang="ts">
import { useModalVisible } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import { Stats } from 'fast-stats';
import { OrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  dealOffset,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { statisColums } from './config';
import { Dayjs } from 'dayjs';

const props = withDefaults(
  defineProps<{
    visible: boolean;
    orders: KungfuApi.OrderResolved[];
    historyDate?: Dayjs;
  }>(),
  {
    visible: false,
    orders: () => [],
    historyDate: () => null,
  },
);

defineEmits<{
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const { modalVisible, closeModal } = useModalVisible(props.visible);
const cancelRatioMean = computed(() => {
  const ordersForStatistic = props.orders.slice(0);
  const cancelRatioBuckets = ordersForStatistic
    .filter((item) => {
      return (
        item.status === OrderStatusEnum.Cancelled ||
        item.status === OrderStatusEnum.PartialFilledNotActive
      );
    })
    .map((item) => {
      return +Number(item.volume_left) / +Number(item.volume);
    });

  if (!cancelRatioBuckets.length) {
    return '--';
  }

  const stats = new Stats().push(...cancelRatioBuckets);
  return stats.amean().toFixed(2);
});

const systemLatencyStats = computed(() => {
  const ordersForStatistic = props.orders.slice(0);
  const systemLatencyBuckets = ordersForStatistic
    .filter((item) => +item.latency_system > 0 && item.latency_system !== '--')
    .map((item) => +item.latency_system);

  if (!systemLatencyBuckets.length) {
    return {
      mean: '--',
      min: '--',
      max: '--',
    };
  }

  const stats = new Stats().push(...systemLatencyBuckets);
  const range = stats.range();
  return {
    mean: stats.amean().toFixed(2),
    min: range[0],
    max: range[1],
  };
});

const networkLatencyStats = computed(() => {
  const ordersForStatistic = props.orders.slice(0);
  const networkLatencyBuckets = ordersForStatistic
    .filter(
      (item) => +item.latency_network > 0 && item.latency_network !== '--',
    )
    .map((item) => +item.latency_network);

  if (!networkLatencyBuckets.length) {
    return {
      mean: '--',
      min: '--',
      max: '--',
    };
  }

  const stats = new Stats().push(...networkLatencyBuckets);
  const range = stats.range();
  return {
    mean: stats.amean().toFixed(2),
    min: range[0],
    max: range[1],
  };
});

const priceVolumeStats = computed(() => {
  const ordersForStatistic = props.orders
    .slice(0)
    .filter((item) => item.limit_price >= 0);
  const priceVolumeData: Record<
    string,
    {
      price: number[];
      volume: number[];
      volumeTraded: number[];
    }
  > = ordersForStatistic.reduce(
    (
      priceVolumeData: Record<
        string,
        {
          price: number[];
          volume: number[];
          volumeTraded: number[];
        }
      >,
      order: KungfuApi.OrderResolved,
    ) => {
      const id = `${order.instrument_id}_${order.exchange_id}_${order.side}_${order.offset}`;
      if (!priceVolumeData[id]) {
        priceVolumeData[id] = {
          price: [],
          volume: [],
          volumeTraded: [],
        };
      }

      priceVolumeData[id].price.push(order.limit_price);
      priceVolumeData[id].volume.push(+Number(order.volume));
      priceVolumeData[id].volumeTraded.push(+Number(order.volume_traded));
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
    const volumeTradedSum = priceVolumeData[id].volumeTraded.reduce(
      (a, b) => a + b,
    );
    const range = priceStats.range();
    return {
      instrumentId_exchangeId: `${instrumentId}_${exchangeId}`,
      side: dealSide(+side),
      offset: dealOffset(+offset),
      mean: priceStats.amean().toFixed(2),
      min: range[0].toFixed(2),
      max: range[1].toFixed(2),
      volume: `${volumeTradedSum} / ${volumeSum}`,
    };
  });

  return priceVolumeDataResolved;
});
</script>
<template>
  <a-modal
    :width="720"
    class="kf-order-statistic-modal"
    v-model:visible="modalVisible"
    :title="`${$t('orderConfig.entrust_statistical')} ${
      !!historyDate ? historyDate.format('YYYY-MM-DD') : $t('orderConfig.statistical_desc')
    }`"
    :destroyOnClose="true"
    :footer="null"
    @cancel="closeModal"
  >
    <a-row style="margin-bottom: 30px">
      <a-col>
        <a-statistic :title="$t('orderConfig.entrust_statistical_number')" :value="orders.length"></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px" class="limit-price-stats-row">
      <div class="title">{{ $t('orderConfig.ntrust_statistical_price') }}</div>
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
      <a-col>
        <a-statistic
          :title="$t('orderConfig.average_withdrawal_ratio')"
          :value="cancelRatioMean"
        ></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px">
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.average_system_latency')"
          :value="systemLatencyStats.mean"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.min_system_latency')"
          :value="systemLatencyStats.min"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.max_system_latency')"
          :value="systemLatencyStats.max"
        ></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px">
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.average_network_latency')"
          :value="networkLatencyStats.mean"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.min_network_latency')"
          :value="networkLatencyStats.min"
        ></a-statistic>
      </a-col>
      <a-col :span="8">
        <a-statistic
          :title="$t('orderConfig.max_network_latency')"
          :value="networkLatencyStats.max"
        ></a-statistic>
      </a-col>
    </a-row>
  </a-modal>
</template>
<style lang="less">
.kf-order-statistic-modal {
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
