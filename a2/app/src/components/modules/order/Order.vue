<script setup lang="ts">
import {
    dealKfPrice,
    dealSide,
    dealOffset,
    dealOrderStatus,
    dealLocationUID,
    getIdByKfLocation,
    delayMilliSeconds,
    dealTradingData,
    getTradingDataSortKey,
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
    CloseSquareOutlined,
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
    dealOrderStat,
    getKungfuHistoryData,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import { UnfinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { OrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings';

const app = getCurrentInstance();

const orders = ref<Order[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Order>(orders, [
    'order_id',
    'instrument_id',
    'exchange_id',
]);
const unfinishedOrder = ref<boolean>(false);
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
            const ordersResolved = (dealTradingData(
                watcher,
                Ledger,
                'Order',
                currentGlobalKfLocation.value,
            ) || []) as Order[];

            if (unfinishedOrder.value) {
                orders.value = toRaw(
                    ordersResolved
                        .slice(0, 100)
                        .filter((item) => !isFinishedOrderStatus(item.status)),
                );
                return;
            }

            orders.value = toRaw(ordersResolved.slice(0, 100));
        });
    }
});

watch(currentGlobalKfLocation, () => {
    historyDate.value = undefined;
    orders.value = [];
});

watch(historyDate, async (newDate) => {
    if (!newDate) {
        Ledger = window.watcher?.ledger;
        return;
    }

    if (currentGlobalKfLocation.value === null) {
        return;
    }

    orders.value = [];
    historyDataLoading.value = true;
    await delayMilliSeconds(500);
    const { tradingData, historyDatas } = await getKungfuHistoryData(
        window.watcher,
        newDate.format(),
        0,
        'Order',
        currentGlobalKfLocation.value,
    );
    Ledger = tradingData;
    orders.value = toRaw(historyDatas as Order[]);
    historyDataLoading.value = false;
});

function isFinishedOrderStatus(orderStatus: OrderStatusEnum): boolean {
    return UnfinishedOrderStatus.indexOf(orderStatus) === -1;
}

function dealLocationUIDResolved(uid: number): string {
    return dealLocationUID(window.watcher, uid);
}

function dealOrderStatResolved(orderUKey: string) {
    return dealOrderStat(Ledger, orderUKey);
}
</script>
<template>
    <div class="kf-orders__warp">
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
                    <a-checkbox size="small" v-model:checked="unfinishedOrder">
                        未完成委托
                    </a-checkbox>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-date-picker v-model:value="historyDate">
                        <template v-if="historyDataLoading" #suffixIcon>
                            <LoadingOutlined />
                        </template>
                    </a-date-picker>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        @click="
                            handleDownload(
                                'Order',
                                currentGlobalKfLocation.value,
                            )
                        "
                    >
                        <template #icon>
                            <DownloadOutlined style="font-size: 14px" />
                        </template>
                    </a-button>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button size="small" type="primary" danger>
                        全部撤单
                    </a-button>
                </KfDashboardItem>
            </template>
            <KfTradingDataTable
                :columns="columns"
                :dataSource="tableData"
                keyField="uid_key"
            >
                <template
                    v-slot:default="{
                        item,
                        column,
                    }: {
                        item: Order,
                        column: KfTradingDataTableHeaderConfig,
                    }"
                >
                    <template v-if="column.dataIndex === 'update_time'">
                        {{ dealKfTime(item.update_time, !!historyDate) }}
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
                    <template v-else-if="column.dataIndex === 'limit_price'">
                        {{ dealKfPrice(item.limit_price) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'volume_traded'">
                        {{ item.volume_traded }} / {{ item.volume }}
                    </template>
                    <template v-else-if="column.dataIndex === 'status'">
                        <span
                            :class="`color-${
                                dealOrderStatus(item.status, item.error_msg)
                                    .color
                            }`"
                        >
                            {{
                                dealOrderStatus(item.status, item.error_msg)
                                    .name
                            }}
                        </span>
                    </template>
                    <template v-else-if="column.dataIndex === 'latency_system'">
                        {{ dealOrderStatResolved(item.uid_key)?.latencySystem }}
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'latency_network'"
                    >
                        {{
                            dealOrderStatResolved(item.uid_key)?.latencyNetwork
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
                    <template v-else-if="column.dataIndex === 'actions'">
                        <CloseSquareOutlined
                            v-if="!isFinishedOrderStatus(item.status)"
                        />
                    </template>
                </template>
            </KfTradingDataTable>
        </KfDashboard>
    </div>
</template>
<script lang="ts">
export default {
    name: '委托记录',
};
</script>
<style lang="less">
.kf-orders__warp {
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
