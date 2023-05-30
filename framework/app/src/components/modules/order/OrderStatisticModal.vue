<script setup lang="ts">
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import {
  useModalVisible,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
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
    isUnfinishedOrder?: boolean;
    historyDate?: null | Dayjs;
  }>(),
  {
    visible: false,
    orders: () => [],
    isUnfinishedOrder: false,
    historyDate: null,
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

interface PriceVolumeStatItem {
  price: number[];
  volume: number[];
  volumeTraded: number[];
  priceByVolume: number[];
}

const priceVolumeStats = computed(() => {
  const ordersForStatistic = props.orders
    .slice(0)
    .filter((item) => item.limit_price > 0);
  const priceVolumeData: Record<string, PriceVolumeStatItem> =
    ordersForStatistic.reduce(
      (
        priceVolumeData: Record<string, PriceVolumeStatItem>,
        order: KungfuApi.OrderResolved,
      ) => {
        const id = `${order.instrument_id}_${order.exchange_id}_${order.side}_${order.offset}`;
        if (!priceVolumeData[id]) {
          priceVolumeData[id] = {
            price: [],
            volume: [],
            volumeTraded: [],
            priceByVolume: [],
          };
        }

        priceVolumeData[id].price.push(order.limit_price);
        priceVolumeData[id].volume.push(+Number(order.volume));
        priceVolumeData[id].volumeTraded.push(
          +Number(order.volume - order.volume_left),
        );
        priceVolumeData[id].priceByVolume.push(
          +Number(order.volume) * order.limit_price,
        );
        return priceVolumeData;
      },
      {} as Record<string, PriceVolumeStatItem>,
    );

  const priceVolumeDataResolved: Array<{
    id: string;
    instrumentId_exchangeId: string;
    side: KungfuApi.KfTradeValueCommonData;
    offset: KungfuApi.KfTradeValueCommonData;
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
      const volumeTradedSum = priceVolumeData[id].volumeTraded.reduce(
        (a, b) => a + b,
      );
      const range = priceStats.range();
      return {
        id,
        instrumentId_exchangeId: `${instrumentId}_${exchangeId}`,
        side: dealSide(+side),
        offset: dealOffset(+offset),
        mean: Number(priceSum / volumeSum).toFixed(2),
        min: range[0].toFixed(2),
        max: range[1].toFixed(2),
        volume: `${volumeTradedSum} / ${volumeSum}`,
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
    class="kf-order-statistic-modal"
    v-model:visible="modalVisible"
    :title="`${$t('orderConfig.entrust_statistical')} ${
      !!historyDate
        ? historyDate.format('YYYY-MM-DD')
        : $t('orderConfig.statistical_desc')
    }`"
    :destroy-on-close="true"
    :footer="null"
    @cancel="closeModal"
  >
    <a-row style="margin-bottom: 30px">
      <a-col>
        <a-statistic
          :title="$t('orderConfig.entrust_statistical_number')"
          :value="orders.length"
        ></a-statistic>
      </a-col>
    </a-row>
    <a-row style="margin-bottom: 30px" class="limit-price-stats-row">
      <div class="title">
        <span>{{ $t('orderConfig.entrust_statistical_price') }}</span>
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
            <template v-if="column.dataIndex === 'side'">
              <span :class="`color-${item.side.color}`">
                {{ item.side.name }}
              </span>
            </template>
            <template v-else-if="column.dataIndex === 'offset'">
              <span :class="`color-${item.offset.color}`">
                {{ item.offset.name }}
              </span>
            </template>
          </template>
        </KfTradingDataTable>
      </div>
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

      display: flex;
      justify-content: space-between;
    }
  }
}
</style>
