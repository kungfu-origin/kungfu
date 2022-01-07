<script setup lang="ts">
import {
    dealKfPrice,
    dealSide,
    dealOffset,
    dealOrderStatus,
    getIdByKfLocation,
    delayMilliSeconds,
    dealTradingData,
    dealOrderStat,
    getCategoryData,
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
    CloseOutlined,
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
    kfCancelAllOrders,
    kfCancelOrder,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import type { Dayjs } from 'dayjs';
import { UnfinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { OrderStatusEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { message, Modal } from 'ant-design-vue';
import { showTradingDataDetail } from '@renderer/assets/methods/actionsUtils';

const app = getCurrentInstance();

const orders = ref<KungfuApi.Order[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.Order>(
    orders,
    ['order_id', 'instrument_id', 'exchange_id'],
);
const unfinishedOrder = ref<boolean>(false);
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
        app.proxy.$tradingDataSubject.subscribe(
            (watcher: KungfuApi.Watcher) => {
                if (historyDate.value) {
                    return;
                }

                if (currentGlobalKfLocation.data === null) {
                    return;
                }

                Ledger = watcher.ledger;
                const ordersResolved = (dealTradingData(
                    watcher,
                    Ledger,
                    'Order',
                    currentGlobalKfLocation.data,
                ) || []) as KungfuApi.Order[];

                if (unfinishedOrder.value) {
                    orders.value = toRaw(
                        ordersResolved
                            .slice(0, 100)
                            .filter(
                                (item) => !isFinishedOrderStatus(item.status),
                            ),
                    );
                    return;
                }

                orders.value = toRaw(ordersResolved.slice(0, 100));
            },
        );
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

    if (currentGlobalKfLocation.data === null) {
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
        currentGlobalKfLocation.data,
    );
    Ledger = tradingData;
    orders.value = toRaw(historyDatas as KungfuApi.Order[]);
    historyDataLoading.value = false;
});

function isFinishedOrderStatus(orderStatus: OrderStatusEnum): boolean {
    return UnfinishedOrderStatus.indexOf(orderStatus) === -1;
}

function dealLocationUIDResolved(
    item: KungfuApi.Order,
    dataIndex: string,
): KungfuApi.KfTradeValueCommonData {
    if (dataIndex === 'source') {
        return resolveAccountId(
            window.watcher,
            item.source,
            item.dest,
            item.parent_id,
        );
    } else {
        return resolveClientId(window.watcher, item.dest, item.parent_id);
    }
}

function dealOrderStatResolved(orderUKey: string): {
    latencySystem: string;
    latencyNetwork: string;
    latencyTrade: string;
    trade_time: bigint;
} | null {
    return dealOrderStat(Ledger, orderUKey);
}

function handleCancelOrder(order: KungfuApi.Order): void {
    const orderId = order.order_id;

    if (!currentGlobalKfLocation.data || !window.watcher) {
        message.error('操作失败');
        return;
    }

    if (currentGlobalKfLocation.data.category === 'strategy') {
        const tdLocation = window.watcher.getLocation(order.source);
        kfCancelOrder(
            window.watcher,
            orderId,
            tdLocation,
            currentGlobalKfLocation.data,
        )
            .then(() => {
                message.success('操作成功');
            })
            .catch(() => {
                message.error('操作失败');
            });
    } else if (currentGlobalKfLocation.data.category === 'td') {
        kfCancelOrder(window.watcher, orderId, currentGlobalKfLocation.data)
            .then(() => {
                message.success('操作成功');
            })
            .catch(() => {
                message.error('操作失败');
            });
    }
}

function handleCancelAllOrders(): void {
    if (!currentGlobalKfLocation.data || !window.watcher) {
        message.error('操作失败');
        return;
    }

    const categoryName = getCategoryData(
        currentGlobalKfLocation.data.category,
    ).name;
    const name = getIdByKfLocation(currentGlobalKfLocation.data);

    Modal.confirm({
        title: '确认全部撤单',
        content: `确认 ${categoryName} ${name} 全部撤单`,
        okText: '确认',
        cancelText: '取消',
        onOk() {
            if (!currentGlobalKfLocation.data || !window.watcher) {
                return;
            }

            return kfCancelAllOrders(
                window.watcher,
                currentGlobalKfLocation.data,
            )
                .then(() => {
                    message.success('操作成功');
                })
                .catch((err) => {
                    message.error('操作失败', err.message);
                });
        },
    });
}

function handleShowTradingDataDetail({
    row,
}: {
    event: MouseEvent;
    row: TradingDataItem;
}) {
    showTradingDataDetail(row as KungfuApi.Order, '委托');
}

resolveClientId;
resolveAccountId;
</script>
<template>
    <div class="kf-orders__warp">
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
                                'Order',
                                currentGlobalKfLocation.data,
                            )
                        "
                    >
                        <template #icon>
                            <DownloadOutlined style="font-size: 14px" />
                        </template>
                    </a-button>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        danger
                        @click="handleCancelAllOrders"
                    >
                        全部撤单
                    </a-button>
                </KfDashboardItem>
            </template>
            <KfTradingDataTable
                :columns="columns"
                :dataSource="tableData"
                keyField="uid_key"
                @rightClickRow="handleShowTradingDataDetail"
            >
                <template
                    v-slot:default="{
                        item,
                        column,
                    }: {
                        item: KungfuApi.Order,
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
                        {{
                            dealOrderStatResolved(item.uid_key)
                                ?.latencySystem || '--'
                        }}
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'latency_network'"
                    >
                        {{
                            dealOrderStatResolved(item.uid_key)
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
                    <template v-else-if="column.dataIndex === 'actions'">
                        <CloseOutlined
                            class="kf-hover"
                            @click="handleCancelOrder(item)"
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
