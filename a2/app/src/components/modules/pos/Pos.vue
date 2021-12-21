<script setup lang="ts">
import {
    LedgerCategoryEnum,
    LedgerCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    dealKfPrice,
    dealAssetPrice,
    dealDirection,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    useCurrentGlobalKfLocation,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@renderer/components/public/KfTradingDataTable.vue';
import { HistoryOutlined } from '@ant-design/icons-vue';

import { getCurrentInstance, onMounted, ref, toRaw } from 'vue';
import { columns } from './config';

const app = getCurrentInstance();

const pos = ref<Position[]>([]);
const { searchKeyword, tableData } = useTableSearchKeyword<Position>(pos, [
    'instrument_id',
    'exchange_id',
    'direction',
]);

const { currentGlobalKfLocation } = useCurrentGlobalKfLocation();

onMounted(() => {
    if (app?.proxy) {
        app.proxy.$tradingDataSubject.subscribe((watcher: Watcher) => {
            if (currentGlobalKfLocation.value === null) {
                return;
            }
            const { category, group, name } = currentGlobalKfLocation.value;
            const ledgerCategory =
                LedgerCategoryEnum[category as LedgerCategoryTypes];
            const positions = watcher.ledger.Position.nofilter(
                'volume',
                BigInt(0),
            ).filter('ledger_category', ledgerCategory);

            const positionsResolved =
                ledgerCategory === 0
                    ? positions
                          .filter('source_id', group)
                          .filter('account_id', name)
                    : positions.filter('client_id', name);

            pos.value = toRaw(
                positionsResolved.sort('instrument_id').reverse(),
            );
        });
    }
});
</script>
<template>
    <div class="kf-position__warp">
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
                        <span :class="dealDirection(item.direction).color">
                            {{ dealDirection(item.direction).name }}
                        </span>
                    </template>
                    <template v-else-if="column.dataIndex === 'today_volume'">
                        {{ item.volume - item.yesterday_volume }}
                    </template>
                    <template v-else-if="column.dataIndex === 'avg_open_price'">
                        {{ dealKfPrice(item.avg_open_price) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'total_price'">
                        {{
                            dealAssetPrice(
                                item.avg_open_price * Number(item.volume),
                            )
                        }}
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'total_market_price'"
                    >
                        {{
                            dealAssetPrice(
                                item.last_price * Number(item.volume),
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'unrealized_pnl'">
                        {{ dealAssetPrice(item.unrealized_pnl) }}
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
