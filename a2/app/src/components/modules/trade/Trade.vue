<script setup lang="ts">
import {
    dealKfPrice,
    dealSide,
    dealOffset,
    dealLocationUID,
    getIdByKfLocation,
    delayMilliSeconds,
    dealTradingData,
    dealOrderStat,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useCurrentGlobalKfLocation,
    useDownloadHistoryTradingData,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@renderer/components/public/KfTradingDataTable.vue';
import {
    DownloadOutlined,
    LoadingOutlined,
    CalendarOutlined,
} from '@ant-design/icons-vue';

import {
    computed,
    getCurrentInstance,
    onMounted,
    ref,
    toRaw,
    watch,
} from 'vue';
import { getColumns } from './config';
import {
    dealKfTime,
    getKungfuHistoryData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';

const app = getCurrentInstance();

const trades = ref<Trade[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Trade>(trades, [
    'order_id',
    'trade_id',
    'instrument_id',
    'exchange_id',
]);
const historyDate = ref<Dayjs>();
const historyDataLoading = ref<boolean>();
var Ledger: TradingData | undefined = window.watcher?.ledger;

const { currentGlobalKfLocation, currentCategoryData, currentUID } =
    useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();

const columns = computed(() => {
    if (currentGlobalKfLocation.value === null) {
        return getColumns('td', !!historyDate.value);
    }

    const { category } = currentGlobalKfLocation.value;
    return getColumns(category, !!historyDate.value);
});

onMounted(() => {
    if (app?.proxy) {
        app.proxy.$tradingDataSubject.subscribe((watcher: Watcher) => {
            if (historyDate.value) {
                return;
            }

            if (currentGlobalKfLocation.value === null) {
                return;
            }

            Ledger = watcher.ledger;
            const tradesResolved = (dealTradingData(
                watcher,
                Ledger,
                'Trade',
                currentGlobalKfLocation.value,
            ) || []) as Trade[];

            trades.value = toRaw(tradesResolved.slice(0, 100));
        });
    }
});

watch(currentGlobalKfLocation, () => {
    historyDate.value = undefined;
    trades.value = [];
});

watch(historyDate, async (newDate) => {
    if (!newDate) {
        Ledger = window.watcher?.ledger;
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
        0,
        'Trade',
        currentGlobalKfLocation.value,
    );
    Ledger = tradingData;
    trades.value = toRaw(historyDatas as Trade[]);
    historyDataLoading.value = false;
});

function dealLocationUIDResolved(uid: number): string {
    return dealLocationUID(window.watcher, uid);
}

function dealOrderStatResolved(order_id: bigint): {
    latencySystem: string;
    latencyNetwork: string;
    latencyTrade: string;
    trade_time: bigint;
} | null {
    const orderUKey = order_id.toString(16).padStart(16, '0');
    return dealOrderStat(Ledger, orderUKey);
}
</script>
<template>
    <div class="kf-trades__warp">
        <KfDashboard>
            <template v-slot:title>
                <span v-if="currentGlobalKfLocation.value">
                    <a-tag
                        v-if="currentCategoryData"
                        :color="currentCategoryData.color"
                    >
                        {{ currentCategoryData.name }}
                    </a-tag>
                    <span class="name" v-if="currentGlobalKfLocation.value">
                        {{
                            getIdByKfLocation(currentGlobalKfLocation.value) ||
                            ''
                        }}
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
                    <a-date-picker v-model:value="historyDate">
                        <template #suffixIcon>
                            <LoadingOutlined v-if="historyDataLoading" />
                            <CalendarOutlined v-else />
                        </template>
                    </a-date-picker>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        @click="
                            handleDownload(
                                'Trade',
                                currentGlobalKfLocation.value,
                            )
                        "
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
            >
                <template
                    v-slot:default="{
                        item,
                        column,
                    }: {
                        item: Trade,
                        column: KfTradingDataTableHeaderConfig,
                    }"
                >
                    <template v-if="column.dataIndex === 'trade_time'">
                        {{ dealKfTime(item.trade_time, !!historyDate) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'kf_time'">
                        {{
                            dealKfTime(
                                dealOrderStatResolved(item.order_id)
                                    ?.trade_time || BigInt(0),
                                !!historyDate,
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'side'">
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
                    <template v-else-if="column.dataIndex === 'latency_trade'">
                        {{
                            dealOrderStatResolved(item.order_id)
                                ?.latencyNetwork || '--'
                        }}
                    </template>
                    <template
                        v-else-if="
                            column.dataIndex === 'source' ||
                            column.dataIndex === 'dest'
                        "
                    >
                        {{ dealLocationUIDResolved(item[column.dataIndex]) }}
                    </template>
                </template>
            </KfTradingDataTable>
        </KfDashboard>
    </div>
</template>
<script lang="ts">
export default {
    name: '成交记录',
};
</script>
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
