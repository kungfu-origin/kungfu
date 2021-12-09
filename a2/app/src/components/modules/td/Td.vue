<script setup lang="ts">
import { ref, reactive, computed, onMounted, getCurrentInstance } from 'vue';
import { storeToRefs } from 'pinia';

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
    KfConfig,
    InstrumentTypeEnum,
    KfLocation,
    ProcessStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings';
import { InstrumentType } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

import { columns } from './config';
import {
    useResetConfigModalPayload,
    useTableSearchKeyword,
    ensureRemoveLocation,
    getExtConfigsRelated,
} from '@renderer/assets/methods/uiUtils';

const app = getCurrentInstance();

const dashboardBodyHeight = ref<number>(0);
const dashboardBodyWidth = ref<number>(0);

const setSourceModalVisible = ref<boolean>(false);
const setTdModalVisible = ref<boolean>(false);
const setTdConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '交易账户',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});
const resetConfigModalPayload = useResetConfigModalPayload();
const currentSelectedSourceId = ref<string>('');

const { extConfigs, extTypeMap } = getExtConfigsRelated();

const tdList = reactive({
    value: [],
});
const tdIdList = computed(() => {
    return tdList.value.map(
        (item: KfConfig): string => `${item.group}_${item.name}`,
    );
});

const tdListResolved = computed(() => {
    return tdList.value.map((item: KfConfig): TdRow => {
        const configValue: Record<string, unknown> = JSON.parse(
            item.value || '{}',
        );
        return {
            accountName:
                (configValue?.account_name as string | undefined) || item.name,
            accountId: item.name,
            sourceId: item.group,
            stateStatus: 'Unknown',
            processStatus: false,
            realizedPnl: 0,
            unrealizedPnl: 0,
            marketValue: 0,
            margin: 0,
            avail: 0,
        };
    });
});

const { searchKeyword, tableData } = useTableSearchKeyword<TdRow>(
    tdListResolved,
    ['sourceId', 'accountId'],
);

onMounted(() => {
    if (app?.proxy) {
        const store = storeToRefs(app?.proxy.$useGlobalStore());
        tdList.value = store.tdList;
    }
});

function handleOpenLog(record: TdRow) {
    console.log(record);
}

function handleSelectedSource(selectedSource: string) {
    setTdModalVisible.value = true;
    setTdConfigPayload.value.title = `${selectedSource} 交易账户`;
    const targetConfig = (extConfigs.value['td'] || {})[selectedSource];
    setTdConfigPayload.value.config = targetConfig;
    currentSelectedSourceId.value = selectedSource;
}

function handleOpenSetSourceDialog() {
    setSourceModalVisible.value = true;
    setTdConfigPayload.value.type = 'add';
}

function handleBodySizeChange({
    width,
    height,
}: {
    width: number;
    height: number;
}) {
    const tableHeaderHeight = 36;
    dashboardBodyHeight.value = height - tableHeaderHeight;
    dashboardBodyWidth.value = width > 800 ? 800 : width;
}

function getTdProcessId(
    sourceId: TdRow['sourceId'],
    accountId: TdRow['accountId'],
): string {
    return `${sourceId}_${accountId}`;
}

function getStateStatusName(processId: string): ProcessStatusTypes {
    processId;
    return 'Unknown';
}

function handleCloseConfigModal() {
    resetConfigModalPayload(setTdConfigPayload);
    currentSelectedSourceId.value = '';
}

function handleRemoveTd(record: TdRow) {
    const tdLocation: KfLocation = {
        category: 'td',
        group: record.sourceId,
        name: record.accountId,
        mode: 'LIVE',
    };
    ensureRemoveLocation(tdLocation).then(() => {
        app?.proxy && app?.proxy.$useGlobalStore().setKfConfigList();
    });
}
</script>

<template>
    <div class="kf-td__warp kf-translateZ">
        <KfDashboard @boardSizeChange="handleBodySizeChange">
            <template v-slot:header>
                <KfDashboardItem>
                    <a-button
                        size="small"
                        type="primary"
                        @click="handleOpenSetSourceDialog"
                    >
                        添加
                    </a-button>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-switch></a-switch>
                </KfDashboardItem>
                <KfDashboardItem>
                    <a-input-search
                        v-model:value="searchKeyword"
                        placeholder="关键字"
                        style="width: 120px"
                    />
                </KfDashboardItem>
            </template>
            <a-table
                ref="table"
                :columns="columns"
                :data-source="tableData"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
            >
                <template
                    #bodyCell="{
                        column,
                        record,
                    }: {
                        column: AntTableColumn,
                        record: TdRow,
                    }"
                >
                    <template v-if="column.dataIndex === 'stateStatus'">
                        <KfProcessStatus
                            :status-name="
                                getStateStatusName(
                                    getTdProcessId(
                                        record.accountId,
                                        record.sourceId,
                                    ),
                                )
                            "
                        ></KfProcessStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'sourceId'">
                        <a-tag
                            :color="
                                InstrumentType[
                                    InstrumentTypeEnum[
                                        extTypeMap[record.sourceId]
                                    ]
                                ].color
                            "
                        >
                            {{ record.sourceId }}
                        </a-tag>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch size="small" :checked="true"></a-switch>
                    </template>
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLog(record)"
                            />
                            <SettingOutlined style="font-size: 12px" />
                            <DeleteOutlined
                                style="font-size: 12px"
                                @click="handleRemoveTd(record)"
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
            @confirm="handleSelectedSource"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setTdModalVisible"
            v-model:visible="setTdModalVisible"
            :payload="setTdConfigPayload"
            :primaryKeyCompareTarget="tdIdList"
            :primaryKeyCompareExtra="currentSelectedSourceId"
            @close="handleCloseConfigModal"
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
