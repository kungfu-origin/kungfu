<script setup lang="ts">
import { ref, computed, toRefs } from 'vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
    FormOutlined,
} from '@ant-design/icons-vue';

import {
    KfCategoryTypes,
    KfExtOriginConfig,
    SetKfConfigPayload,
} from '@kungfu-trader/kungfu-js-api/typings';
import type {
    KfConfig,
    KfConfigValue,
} from '@kungfu-trader/kungfu-js-api/typings';

import {
    useTableSearchKeyword,
    useAllKfConfigData,
    useDashboardBodySize,
    handleOpenLogview,
    useProcessStatusDetailData,
    useAssets,
    useCurrentGlobalKfLocation,
} from '@renderer/assets/methods/uiUtils';
import { columns } from './config';
import {
    handleSwitchProcessStatus,
    useAddUpdateRemoveKfConfig,
    useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';
import {
    dealAssetPrice,
    getConfigValue,
    getIfProcessRunning,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import path from 'path';
import { shell } from '@electron/remote';

interface StrategyProps {}
defineProps<StrategyProps>();

const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
    useDashboardBodySize();

const setStrategyModalVisible = ref<boolean>(false);
const setStrategyConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '策略',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});

const { strategy } = toRefs(useAllKfConfigData());
const strategyIdList = computed(() => {
    return strategy.value.map((item: KfConfig): string => item.name);
});

const { dealRowClassName, customRow } = useCurrentGlobalKfLocation();

const { processStatusData } = useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
    strategy,
    processStatusData,
);
const { searchKeyword, tableData } = useTableSearchKeyword<KfConfig>(strategy, [
    'name',
]);
const { getAssetsByKfConfig } = useAssets();

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
    useAddUpdateRemoveKfConfig();

function handleOpenSetStrategyDialog(
    type: ModalChangeType,
    strategyConfig?: KfConfig,
) {
    setStrategyConfigPayload.value.type = type;
    setStrategyConfigPayload.value.config = {
        type: [],
        settings: [
            {
                key: 'strategy_id',
                name: '策略ID',
                type: 'str',
                primary: true,
                required: true,
                tip: '需保证该策略ID唯一',
            },
            {
                key: 'strategy_path',
                name: '策略路径',
                type: 'file',
                required: true,
            },
        ],
    };

    if (type === 'update') {
        if (strategyConfig) {
            setStrategyConfigPayload.value.initValue = JSON.parse(
                strategyConfig.value,
            );
        }
    }

    setStrategyModalVisible.value = true;
}

function getStrategyPathShowName(kfConfig: KfConfig): string {
    const strategyPath = getConfigValue(kfConfig).strategy_path || '';
    return path.basename(strategyPath);
}

function handleOpenFile(kfConfig: KfConfig) {
    const strategyPath = getConfigValue(kfConfig).strategy_path || '';
    shell.openPath(strategyPath);
}
</script>

<template>
    <div class="kf-strategy__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-switch
                        :checked="allProcessOnline"
                        @click="handleSwitchAllProcessStatus"
                    ></a-switch>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleOpenSetStrategyDialog('add')"
                    >
                        添加
                    </a-button>
                </KfDashboardItem>
            </template>
            <a-table
                :columns="columns"
                :data-source="tableData"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
                :rowClassName="dealRowClassName"
                :customRow="customRow"
                emptyText="暂无数据"
            >
                <template
                    #bodyCell="{
                        column,
                        record,
                    }: {
                        column: AntTableColumn,
                        record: KfConfig,
                    }"
                >
                    <template v-if="column.dataIndex === 'strategyFile'">
                        {{ getStrategyPathShowName(record) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch
                            size="small"
                            :checked="
                                getIfProcessRunning(
                                    processStatusData,
                                    getProcessIdByKfLocation(record),
                                )
                            "
                            @click="handleSwitchProcessStatus($event, record)"
                        ></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'unrealizedPnl'">
                        {{
                            dealAssetPrice(
                                getAssetsByKfConfig(record).unrealized_pnl,
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'avail'">
                        {{ dealAssetPrice(getAssetsByKfConfig(record).avail) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'marketValue'">
                        {{
                            dealAssetPrice(
                                getAssetsByKfConfig(record).market_value,
                            )
                        }}
                    </template>
                    <template v-else-if="column.dataIndex === 'margin'">
                        {{ dealAssetPrice(getAssetsByKfConfig(record).margin) }}
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLogview(record)"
                            />
                            <FormOutlined
                                style="font-size: 12px"
                                @click="handleOpenFile(record)"
                            ></FormOutlined>
                            <SettingOutlined
                                style="font-size: 12px"
                                @click="
                                    handleOpenSetStrategyDialog(
                                        'update',
                                        record,
                                    )
                                "
                            />
                            <DeleteOutlined
                                style="font-size: 12px"
                                @click="handleRemoveKfConfig(record)"
                            />
                        </div>
                    </template>
                </template>
            </a-table>
        </KfDashboard>
        <KfSetByConfigModal
            v-if="setStrategyModalVisible"
            :width="420"
            v-model:visible="setStrategyModalVisible"
            :payload="setStrategyConfigPayload"
            :primaryKeyAvoidRepeatCompareTarget="strategyIdList"
            @confirm="
                handleConfirmAddUpdateKfConfig($event, 'strategy', 'default')
            "
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
export default {
    name: '策略进程',
};
</script>

<style lang="less">
.kf-strategy__warp {
    height: 100%;
}
</style>
