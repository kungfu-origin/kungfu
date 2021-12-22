<script setup lang="ts">
import {
    dealKfPrice,
    dealSide,
    dealOffset,
    dealLocationUID,
    getIdByKfLocation,
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
import { dealKfTime } from '@kungfu-trader/kungfu-js-api/kungfu';

const app = getCurrentInstance();

const trades = ref<Trade[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Trade>(trades, [
    'order_id',
    'trade_id',
    'instrument_id',
    'exchange_id',
]);

const { currentGlobalKfLocation, currentCategoryData } =
    useCurrentGlobalKfLocation();

const columns = computed(() => {
    if (currentGlobalKfLocation.value === null) {
        return getColumns('td');
    }

    const { category } = currentGlobalKfLocation.value;
    return getColumns(category);
});

const tradeFilterKey = computed(() => {
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

            const tradesResolved = watcher.ledger.Trade.filter(
                tradeFilterKey.value,
                currentUID,
            );

            trades.value = toRaw(tradesResolved.sort('update_time'));
        });
    }
});

function dealLocationUIDResolved(uid: number): string {
    return dealLocationUID(window.watcher, uid);
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
                    <HistoryOutlined style="font-size: 14px" />
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
                        {{ dealKfTime(item.trade_time) }}
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
                    <template v-else-if="column.dataIndex === 'price'">
                        {{ dealKfPrice(item.price) }}
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
