<script setup lang="ts">
import { computed, ref, toRefs } from 'vue';
import {
    FileTextOutlined,
    SettingOutlined,
    DeleteOutlined,
} from '@ant-design/icons-vue';

import KfDashboard from '@renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@renderer/components/public/KfDashboardItem.vue';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import KfSetSourceModal from '@renderer/components/public/KfSetSourceModal.vue';
import KfSetByConfigModal from '@renderer/components/public/KfSetByConfigModal.vue';

import {
    KfExtOriginConfig,
    SetKfConfigPayload,
} from '@kungfu-trader/kungfu-js-api/typings';
import type { KfConfigValue } from '@kungfu-trader/kungfu-js-api/typings';
import { columns } from './config';
import {
    getInstrumentTypeColor,
    useExtConfigsRelated,
    useProcessStatusDetailData,
    handleOpenLogview,
    useDashboardBodySize,
    useTableSearchKeyword,
    useAllKfConfigData,
} from '@renderer/assets/methods/uiUtils';
import {
    getIdByKfLocation,
    getIfProcessRunning,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    handleSwitchProcessStatus,
    useAddUpdateRemoveKfConfig,
    useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';

interface MdProps {}
defineProps<MdProps>();

const { dashboardBodyHeight, dashboardBodyWidth, handleBodySizeChange } =
    useDashboardBodySize();

const setSourceModalVisible = ref<boolean>(false);
const setMdModalVisible = ref<boolean>(false);
const setMdConfigPayload = ref<SetKfConfigPayload>({
    type: 'add',
    title: '行情源',
    config: {} as KfExtOriginConfig['config'][KfCategoryTypes],
});
const currentSelectedSourceId = ref<string>('');

const { extConfigs, extTypeMap } = useExtConfigsRelated();
const { md } = toRefs(useAllKfConfigData());
const mdIdList = computed(() => {
    return md.value.map((item: KfConfig): string => getIdByKfLocation(item));
});
const { processStatusData, getProcessStatusName } =
    useProcessStatusDetailData();
const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
    md,
    processStatusData,
);

const { searchKeyword, tableData } = useTableSearchKeyword<KfConfig>(md, [
    'group',
]);

const { handleConfirmAddUpdateKfConfig, handleRemoveKfConfig } =
    useAddUpdateRemoveKfConfig();

function handleOpenSetMdDialog(
    type = 'add' as ModalChangeType,
    selectedSource: string,
    mdConfig?: KfConfig,
) {
    currentSelectedSourceId.value = selectedSource;
    setMdConfigPayload.value.type = type;
    setMdConfigPayload.value.title = `${selectedSource} 行情源`;
    setMdConfigPayload.value.config = (extConfigs.value['md'] || {})[
        selectedSource
    ];

    if (type === 'update') {
        if (mdConfig) {
            setMdConfigPayload.value.initValue = JSON.parse(mdConfig.value);
        }
    }

    if (!setMdConfigPayload.value.config?.settings?.length) {
        handleConfirmAddUpdateKfConfig(
            {
                formState: {} as Record<string, KfConfigValue>,
                idByPrimaryKeys: selectedSource,
                changeType: type,
            },
            'md',
            selectedSource,
        );
        return;
    }

    setMdModalVisible.value = true;
}

function handleOpenSetSourceDialog() {
    setSourceModalVisible.value = true;
}
</script>
<template>
    <div class="kf-md__warp kf-translateZ">
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
                    <KfDashboardItem>
                        <a-switch
                            :checked="allProcessOnline"
                            @click="handleSwitchAllProcessStatus"
                        ></a-switch>
                    </KfDashboardItem>
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
                ref="table"
                :columns="columns"
                :data-source="tableData"
                size="small"
                :pagination="false"
                :scroll="{ y: dashboardBodyHeight - 4, x: dashboardBodyWidth }"
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
                    <template v-if="column.dataIndex === 'name'">
                        <a-tag
                            :color="
                                getInstrumentTypeColor(extTypeMap[record.name])
                            "
                        >
                            {{ record.group }}
                        </a-tag>
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
                    <template v-else-if="column.dataIndex === 'actions'">
                        <div class="kf-actions__warp">
                            <FileTextOutlined
                                style="font-size: 12px"
                                @click="handleOpenLogview(record)"
                            />
                            <SettingOutlined
                                style="font-size: 12px"
                                @click="
                                    handleOpenSetMdDialog(
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
            sourceType="md"
            @confirm="handleOpenSetMdDialog('add', $event)"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setMdModalVisible"
            v-model:visible="setMdModalVisible"
            :payload="setMdConfigPayload"
            :primaryKeyAvoidRepeatCompareTarget="mdIdList"
            :primaryKeyAvoidRepeatCompareExtra="currentSelectedSourceId"
            @confirm="
                handleConfirmAddUpdateKfConfig(
                    { ...$event, idByPrimaryKeys: currentSelectedSourceId },
                    'md',
                    currentSelectedSourceId,
                )
            "
        ></KfSetByConfigModal>
    </div>
</template>

<script lang="ts">
export default {
    name: '行情源',
};
</script>

<style lang="less">
.kf-md__warp {
    height: 100%;
}
</style>
