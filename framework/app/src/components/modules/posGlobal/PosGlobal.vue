<script setup lang="ts">
import {
    useDashboardBodySize,
    useTableSearchKeyword,
} from '@renderer/assets/methods/uiUtils';
import {
    getCurrentInstance,
    onBeforeUnmount,
    onMounted,
    ref,
    toRaw,
} from 'vue';
import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfBlinkNum from '@renderer/components/public/KfBlinkNum.vue';
import { columns } from './config';
import {
    dealAssetPrice,
    dealDirection,
    dealKfPrice,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';

interface PositionGlobalProps {}
defineProps<PositionGlobalProps>();

const app = getCurrentInstance();
const pos = ref<KungfuApi.Position[]>([]);
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { searchKeyword, tableData } = useTableSearchKeyword<KungfuApi.Position>(
    pos,
    ['instrument_id', 'exchange_id', 'direction'],
);

onMounted(() => {
    if (app?.proxy) {
        const subscription = app.proxy.$tradingDataSubject.subscribe(
            (watcher: KungfuApi.Watcher) => {
                const now = new Date().getTime();
                console.log(now, '-----');
                const positions = watcher.ledger.Position.filter(
                    'ledger_category',
                    LedgerCategoryEnum.td,
                ).list();
                pos.value = toRaw(buildGlobalPositions(positions));
            },
        );

        onBeforeUnmount(() => {
            subscription.unsubscribe();
        });
    }
});

type PosStat = Record<string, KungfuApi.Position>;

function buildGlobalPositions(
    positions: KungfuApi.Position[],
): KungfuApi.Position[] {
    const posStatData: PosStat = positions.reduce((posStat, pos) => {
        const id = `${pos.exchange_id}_${pos.instrument_id}`;
        if (!posStat[id]) {
            posStat[id] = pos;
        } else {
            const prePosStat = posStat[id];
            const { avg_open_price, volume, yesterday_volume, unrealized_pnl } =
                prePosStat;
            posStat[id] = {
                ...prePosStat,
                yesterday_volume: yesterday_volume + pos.yesterday_volume,
                volume: volume + pos.volume,

                avg_open_price:
                    (avg_open_price * Number(volume) +
                        pos.avg_open_price * Number(pos.volume)) /
                    (Number(pos.volume) + Number(pos.volume)),
                unrealized_pnl: unrealized_pnl + pos.unrealized_pnl,
            };
        }
        return posStat;
    }, {} as PosStat);

    return Object.values(posStatData).sort((item1, item2) => {
        const id1 = `${item1.instrument_id}_${item1.instrument_id}`;
        const id2 = `${item2.instrument_id}_${item2.instrument_id}`;
        return id1.localeCompare(id2);
    });
}
</script>
<template>
    <div class="kf-position-global__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
            </template>
            <a-table
                :columns="columns"
                :data-source="tableData"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4 }"
                emptyText="暂无数据"
            >
                <template
                    #bodyCell="{
                        column,
                        record,
                    }: {
                        column: AntTableColumn,
                        record: KungfuApi.Position,
                    }"
                >
                    <template v-if="column.dataIndex === 'instrument_id'">
                        {{ record.instrument_id }}
                        {{ record.exchange_id }}
                    </template>
                    <template v-else-if="column.dataIndex === 'direction'">
                        <span
                            :class="`color-${
                                dealDirection(record.direction).color
                            }`"
                        >
                            {{ dealDirection(record.direction).name }}
                        </span>
                    </template>
                    <template
                        v-else-if="column.dataIndex === 'yesterday_volume'"
                    >
                        <KfBlinkNum
                            :num="Number(record.yesterday_volume).toFixed(0)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'today_volume'">
                        <KfBlinkNum
                            :num="
                                Number(
                                    record.volume - record.yesterday_volume,
                                ).toFixed(0)
                            "
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'volume'">
                        <KfBlinkNum
                            :num="Number(record.volume).toFixed(0)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'avg_open_price'">
                        <KfBlinkNum
                            :num="dealKfPrice(record.avg_open_price)"
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'total_price'">
                        <KfBlinkNum
                            :num="
                                dealAssetPrice(
                                    record.avg_open_price *
                                        Number(record.volume),
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
                                    record.last_price * Number(record.volume),
                                )
                            "
                        ></KfBlinkNum>
                    </template>
                    <template v-else-if="column.dataIndex === 'unrealized_pnl'">
                        <KfBlinkNum
                            mode="compare-zero"
                            :num="dealAssetPrice(record.unrealized_pnl)"
                        ></KfBlinkNum>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
    </div>
</template>
<style lang="less">
.kf-position-global__warp {
    height: 100%;
}
</style>
