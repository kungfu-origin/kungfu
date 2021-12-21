<script setup lang="ts">
import {
    dealKfPrice,
    dealAssetPrice,
    dealSide,
    dealOffset,
    dealOrderStatus,
    dealLocationUID,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useCurrentGlobalKfLocation,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@renderer/components/public/KfTradingDataTable.vue';
import { HistoryOutlined } from '@ant-design/icons-vue';

import { computed, getCurrentInstance, onMounted, ref, toRaw } from 'vue';
import { getColumns } from './config';
import {
    Offset,
    Side,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';

const app = getCurrentInstance();

const orders = ref<Order[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Order>(orders, [
    'order_id',
    'instrument_id',
    'exchange_id',
]);

const { currentGlobalKfLocation } = useCurrentGlobalKfLocation();

const columns = computed(() => {
    if (currentGlobalKfLocation.value === null) {
        return getColumns('td');
    }

    const { category } = currentGlobalKfLocation.value;
    return getColumns(category);
});

const orderFilterKey = computed(() => {
    if (currentGlobalKfLocation.value === null) {
        return '';
    }

    const { category } = currentGlobalKfLocation.value;
    if (category === 'td') {
        return 'source';
    } else if (category === 'strategy') {
        return 'dest';
    }

    return '';
});

onMounted(() => {
    if (app?.proxy) {
        app.proxy.$tradingDataSubject.subscribe((watcher: Watcher) => {
            if (currentGlobalKfLocation.value === null) {
                return;
            }

            const currentUID = watcher.getLocationUID(
                currentGlobalKfLocation.value,
            );

            orders.value = watcher.ledger.Order.filter(
                orderFilterKey.value,
                currentUID,
            )
                .sort('update_time')
                .slice(0, 100);
        });
    }
});

function dealLocationUIDResolved(uid: number): string {
    return dealLocationUID(window.watcher, uid);
}
</script>
<template>
    <div class="kf-orders__warp">
        <KfDashboard>
            <template v-slot:header>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
                <KfDashboardItem>
                    <HistoryOutlined style="font-size: 14px" />
                </KfDashboardItem>
            </template>
            <KfTradingDataTable
                :columns="columns"
                :dataSource="tableData"
                keyField="order_id"
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
                        {{ dealKfTime(item.update_time) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'side'">
                        <span :class="dealSide(item.side).color">
                            {{ dealSide(item.side).name }}
                        </span>
                    </template>
                    <template v-else-if="column.dataIndex === 'offset'">
                        <span :class="dealOffset(item.offset).color">
                            {{ dealOffset(item.offset).name }}
                        </span>
                    </template>
                    <template v-else-if="column.dataIndex === 'volume_traded'">
                        {{ item.volume_traded }} / {{ item.volume }}
                    </template>
                    <template v-else-if="column.dataIndex === 'status'">
                        <span
                            :class="
                                dealOrderStatus(item.status, item.error_msg)
                                    .color
                            "
                        >
                            {{
                                dealOrderStatus(item.status, item.error_msg)
                                    .name
                            }}
                        </span>
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
