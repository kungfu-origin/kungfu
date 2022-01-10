<script setup lang="ts">
import {
    dealKfPrice,
    dealSide,
    dealOffset,
    getIdByKfLocation,
    delayMilliSeconds,
    dealTradingData,
    dealOrderStat,
    resolveAccountId,
    resolveClientId,
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
    onBeforeUnmount,
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
import { showTradingDataDetail } from '@renderer/assets/methods/actionsUtils';

const app = getCurrentInstance();

const trades = ref<KungfuApi.Trade[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.Trade>(
    trades,
    ['order_id', 'trade_id', 'instrument_id', 'exchange_id'],
);
const historyDate = ref<Dayjs>();
const historyDataLoading = ref<boolean>();
var Ledger: KungfuApi.TradingData | undefined = window.watcher?.ledger;

const {
    currentGlobalKfLocation,
    currentCategoryData,
    getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();

const columns = computed(() => {
    if (currentGlobalKfLocation.data === null) {
        return getColumns('td', !!historyDate.value);
    }

    const { category } = currentGlobalKfLocation.data;
    return getColumns(category, !!historyDate.value);
});

onMounted(() => {
    if (app?.proxy) {
        const subscription = app.proxy.$tradingDataSubject.subscribe(
            (watcher: KungfuApi.Watcher) => {
                if (historyDate.value) {
                    return;
                }

                if (currentGlobalKfLocation.data === null) {
                    return;
                }

                Ledger = watcher.ledger;
                const tradesResolved = (dealTradingData(
                    watcher,
                    Ledger,
                    'Trade',
                    currentGlobalKfLocation.data,
                ) || []) as KungfuApi.Trade[];

                trades.value = toRaw(tradesResolved.slice(0, 100));
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
        Ledger = window.watcher?.ledger;
        return;
    }

    if (currentGlobalKfLocation.data === null) {
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
        currentGlobalKfLocation.data,
    );
    Ledger = tradingData;
    trades.value = toRaw(historyDatas as KungfuApi.Trade[]);
    historyDataLoading.value = false;
});

function dealLocationUIDResolved(
    item: KungfuApi.Trade,
    dataIndex: string,
): KungfuApi.KfTradeValueCommonData {
    if (dataIndex === 'source') {
        return resolveAccountId(
            window.watcher,
            item.source,
            item.dest,
            item.parent_order_id,
        );
    } else {
        return resolveClientId(window.watcher, item.dest, item.parent_order_id);
    }
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

function handleShowTradingDataDetail({
    row,
}: {
    event: MouseEvent;
    row: TradingDataItem;
}) {
    showTradingDataDetail(row as KungfuApi.Trade, '成交');
}
</script>
<template>
    <div class="kf-trades__warp kf-translateZ">
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
                        {{
                            getCurrentGlobalKfLocationId(
                                currentGlobalKfLocation.data,
                            )
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
                                currentGlobalKfLocation.data,
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
                @rightClickRow="handleShowTradingDataDetail"
            >
                <template
                    v-slot:default="{
                        item,
                        column,
                    }: {
                        item: KungfuApi.Trade,
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
                        <span
                            :class="[
                                `color-${
                                    dealLocationUIDResolved(
                                        item,
                                        column.dataIndex,
                                    ).color
                                }`,
                            ]"
                        >
                            {{
                                dealLocationUIDResolved(item, column.dataIndex)
                                    .name
                            }}
                        </span>
                    </template>
                </template>
            </KfTradingDataTable>
        </KfDashboard>
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
