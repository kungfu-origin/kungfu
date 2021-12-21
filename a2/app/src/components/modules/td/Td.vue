<script setup lang="ts">
import { ref, computed, toRefs } from 'vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import KfSetSourceModal from '@renderer/components/public/KfSetSourceModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
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
import { columns } from './config';
import {
    useTableSearchKeyword,
    useExtConfigsRelated,
    useAllKfConfigData,
    useProcessStatusDetailData,
    handleOpenLogview,
    useDashboardBodySize,
    getInstrumentTypeColor,
    useAssets,
    useCurrentGlobalKfLocation,
} from '@renderer/assets/methods/uiUtils';
import {
    useAddUpdateRemoveKfConfig,
    handleSwitchProcessStatus,
    useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';
import {
    dealAssetPrice,
    getIfProcessRunning,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { message } from 'ant-design-vue';

interface TdProps {}
defineProps<TdProps>();

const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
    useDashboardBodySize();

const setSourceModalVisible = ref<boolean>(false);
const setTdModalVisible = ref<boolean>(false);
const setTdConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '交易账户',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});

const currentSelectedSourceId = ref<string>('');
const { extConfigs, extTypeMap } = useExtConfigsRelated();
const { td } = toRefs(useAllKfConfigData());
const tdIdList = computed(() => {
    return td.value.map(
        (item: KfConfig): string => `${item.group}_${item.name}`,
    );
});
const { dealRowClassName, customRow } = useCurrentGlobalKfLocation();

const { processStatusData, getProcessStatusName } =
    useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
    td,
    processStatusData,
);
const { searchKeyword, tableData } = useTableSearchKeyword<KfConfig>(td, [
    'group',
    'name',
]);
const { getAssetsByKfConfig } = useAssets();

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
    useAddUpdateRemoveKfConfig();

function handleOpenSetTdDialog(
    type = 'add' as ModalChangeType,
    selectedSource: string,
    tdConfig?: KfConfig,
) {
    currentSelectedSourceId.value = selectedSource;
    setTdConfigPayload.value.type = type;
    setTdConfigPayload.value.title = `${selectedSource} 交易账户`;
    setTdConfigPayload.value.config = (extConfigs.value['td'] || {})[
        selectedSource
    ];

    if (type === 'update') {
        if (tdConfig) {
            setTdConfigPayload.value.initValue = JSON.parse(tdConfig.value);
        }
    }

    if (!setTdConfigPayload.value.config?.settings?.length) {
        message.error(`配置项不存在, 请检查 ${selectedSource} package.json`);
        return;
    }

    setTdModalVisible.value = true;
}

function handleOpenSetSourceDialog() {
    setSourceModalVisible.value = true;
}
</script>

<template>
    <div class="kf-td__warp kf-translateZ">
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
                        @click="handleOpenSetSourceDialog"
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
                    <template v-if="column.dataIndex === 'accountName'">
                        {{ JSON.parse(record.value).account_name || '--' }}
                    </template>
                    <template v-else-if="column.dataIndex === 'name'">
                        <a-tag
                            :color="
                                getInstrumentTypeColor(extTypeMap[record.group])
                            "
                        >
                            {{ record.group }}
                        </a-tag>
                        {{ record.name }}
                    </template>
                    <template v-else-if="column.dataIndex === 'stateStatus'">
                        <KfProcessStatus
                            :statusName="getProcessStatusName(record)"
                        ></KfProcessStatus>
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
                            <SettingOutlined
                                style="font-size: 12px"
                                @click="
                                    handleOpenSetTdDialog(
                                        'update',
                                        record.group,
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
        <KfSetSourceModal
            v-if="setSourceModalVisible"
            v-model:visible="setSourceModalVisible"
            sourceType="td"
            @confirm="handleOpenSetTdDialog('add', $event)"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setTdModalVisible"
            v-model:visible="setTdModalVisible"
            :payload="setTdConfigPayload"
            :primaryKeyAvoidRepeatCompareTarget="tdIdList"
            :primaryKeyAvoidRepeatCompareExtra="currentSelectedSourceId"
            @confirm="
                handleConfirmAddUpdateKfConfig(
                    $event,
                    'td',
                    currentSelectedSourceId,
                )
            "
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
export default {
    name: '交易账户',
};
</script>

<style lang="less">
.kf-td__warp {
    height: 100%;
}
</style>
