<script setup lang="ts">
import {
    dealKfPrice,
    dealAssetPrice,
    dealDirection,
    getIdByKfLocation,
    dealTradingData,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useCurrentGlobalKfLocation,
    useDownloadHistoryTradingData,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@renderer/components/public/KfTradingDataTable.vue';
import { DownloadOutlined } from '@ant-design/icons-vue';

import { getCurrentInstance, onMounted, ref, toRaw } from 'vue';
import { columns } from './config';
import KfBlinkNum from '@renderer/components/public/KfBlinkNum.vue';

const app = getCurrentInstance();

const pos = ref<Position[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Position>(pos, [
    'instrument_id',
    'exchange_id',
    'direction',
]);

const { currentGlobalKfLocation, currentCategoryData } =
    useCurrentGlobalKfLocation(window.watcher);

const { handleDownload } = useDownloadHistoryTradingData();

onMounted(() => {
    if (app?.proxy) {
        app.proxy.$tradingDataSubject.subscribe((watcher: Watcher) => {
            if (currentGlobalKfLocation.value === null) {
                return;
            }

            const positions = (dealTradingData(
                window.watcher,
                watcher.ledger,
                'Position',
                currentGlobalKfLocation.value,
            ) || []) as Position[];

            pos.value = toRaw(positions.reverse());
        });
    }
});
</script>
<template>
    <div class="kf-position__warp">
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
                    <a-button
                        size="small"
                        @click="
                            handleDownload(
                                'Position',
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
                keyField="uid_key"
            >
                <template
                    v-slot:default="{
                        item,
                        column,
                    }: {
                        item: Position,
                        column: KfTradingDataTableHeaderConfig,
                    }"
                >
                    <template v-if="column.dataIndex === 'instrument_id'">
                        {{ item.instrument_id }}
                        {{ item.exchange_id }}
                    </template>
                    <template v-else-if="column.dataIndex === 'direction'">
                        <span
                            :class="`color-${
                                dealDirection(item.direction).color
                            }`"
                        >
                            {{ dealDirection(item.direction).name }}
                        </span>
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'yesterday_volume'"
                    >
                        <KfBlinkNum
                            :num="Number(item.yesterday_volume).toFixed(0)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'today_volume'">
                        <KfBlinkNum
                            :num="
                                Number(
                                    item.volume - item.yesterday_volume,
                                ).toFixed(0)
                            "
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'volume'">
                        <KfBlinkNum
                            :num="Number(item.volume).toFixed(0)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'avg_open_price'">
                        <KfBlinkNum
                            :num="dealKfPrice(item.avg_open_price)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'total_price'">
                        <KfBlinkNum
                            :num="
                                dealAssetPrice(
                                    item.avg_open_price * Number(item.volume),
                                )
                            "
                        ></KfBlinkNum>
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'total_market_price'"
                    >
                        <KfBlinkNum
                            :num="
                                dealAssetPrice(
                                    item.last_price * Number(item.volume),
                                )
                            "
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'unrealized_pnl'">
                        <KfBlinkNum
                            :num="dealAssetPrice(item.unrealized_pnl)"
                        ></KfBlinkNum>
                    </template>
                </template>
            </KfTradingDataTable>
        </KfDashboard>
    </div>
</template>
<script lang="ts">
export default {
    name: '持仓',
};
</script>
<style lang="less">
.kf-position__warp {
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
