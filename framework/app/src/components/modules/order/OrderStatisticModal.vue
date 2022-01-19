<script setup lang="ts">
import { UnfinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { useModalVisible } from '@renderer/assets/methods/uiUtils';
import { computed } from 'vue';
import { Stats } from 'fast-stats';
import { OrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
    dealOffset,
    dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { statisColums } from './config';

const props = withDefaults(
    defineProps<{
        visible: boolean;
        orders: KungfuApi.OrderResolved[];
        isHistroy?: boolean;
    }>(),
    {
        visible: false,
        orders: () => [],
        isHistroy: false,
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
                UnfinishedOrderStatus.indexOf(+item.status) === -1 &&
                item.status !== OrderStatusEnum.Lost
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
        .filter(
            (item) => +item.latency_system > 0 && item.latency_system !== '--',
        )
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
            (item) =>
                +item.latency_network > 0 && item.latency_network !== '--',
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

const limitPriceStats = computed(() => {
    const ordersForStatistic = props.orders
        .slice(0)
        .filter((item) => item.limit_price > 0);
    const limitPriceData: Record<string, number[]> = ordersForStatistic.reduce(
        (limitPriceData: Record<string, number[]>, order: KungfuApi.Order) => {
            const id = `${order.instrument_id}_${order.exchange_id}_${order.side}_${order.offset}`;
            if (!limitPriceData[id]) {
                limitPriceData[id] = [];
            }

            limitPriceData[id].push(order.limit_price);
            return limitPriceData;
        },
        {} as Record<string, number[]>,
    );

    const limitPriceDataResolved: Array<{
        instrumentId_exchangeId: string;
        side: KungfuApi.KfTradeValueCommonData;
        offset: KungfuApi.KfTradeValueCommonData;
        mean: string;
        min: string;
        max: string;
    }> = Object.keys(limitPriceData).map((id) => {
        const [instrumentId, exchangeId, side, offset] = id.split('_');
        const stats = new Stats().push(...limitPriceData[id]);
        const range = stats.range();
        return {
            instrumentId_exchangeId: `${instrumentId}_${exchangeId}`,
            side: dealSide(+side),
            offset: dealOffset(+offset),
            mean: stats.amean().toFixed(2),
            min: range[0].toFixed(2),
            max: range[1].toFixed(2),
        };
    });

    return limitPriceDataResolved;
});
</script>
<template>
    <a-modal
        :width="720"
        class="kf-order-statistic-modal"
        v-model:visible="modalVisible"
        :title="`委托统计 ${isHistroy ? '(历史)' : ''}`"
        :destroyOnClose="true"
        :footer="null"
        @cancel="closeModal"
    >
        <a-row style="margin-bottom: 30px" class="limit-price-stats-row">
            <div class="title">委托价统计</div>
            <a-table
                v-if="limitPriceStats"
                size="small"
                :dataSource="limitPriceStats"
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
                    title="平均撤单比 (仅统计 部成部撤 和 全部撤单)"
                    :value="cancelRatioMean"
                ></a-statistic>
            </a-col>
        </a-row>
        <a-row style="margin-bottom: 30px">
            <a-col :span="8">
                <a-statistic
                    title="平均系统延迟(μs)"
                    :value="systemLatencyStats.mean"
                ></a-statistic>
            </a-col>
            <a-col :span="8">
                <a-statistic
                    title="最小系统延迟(μs)"
                    :value="systemLatencyStats.min"
                ></a-statistic>
            </a-col>
            <a-col :span="8">
                <a-statistic
                    title="最大系统延迟(μs)"
                    :value="systemLatencyStats.max"
                ></a-statistic>
            </a-col>
        </a-row>
        <a-row style="margin-bottom: 30px">
            <a-col :span="8">
                <a-statistic
                    title="平均网络延迟(μs)"
                    :value="networkLatencyStats.mean"
                ></a-statistic>
            </a-col>
            <a-col :span="8">
                <a-statistic
                    title="最小网络延迟(μs)"
                    :value="networkLatencyStats.min"
                ></a-statistic>
            </a-col>
            <a-col :span="8">
                <a-statistic
                    title="最大网络延迟(μs)"
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
