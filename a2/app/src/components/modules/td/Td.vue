<script setup lang="ts">
import { ref, computed, getCurrentInstance, toRefs } from 'vue';

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
    ProcessStatusTypes,
} from '@kungfu-trader/kungfu-js-api/typings';
import type { KfConfig } from '@kungfu-trader/kungfu-js-api/typings';
import { columns } from './config';
import {
    useTableSearchKeyword,
    getExtConfigsRelated,
    getAllKfConfigData,
    getProcessStatusDetailData,
    handleOpenLogview,
    useDashboardBodySize,
    getExtColor,
} from '@renderer/assets/methods/uiUtils';
import {
    handleRemoveKfConfig,
    handleSwitchProcessStatus,
    useSwitchAllConfig,
} from '@renderer/assets/methods/actionsUtils';
import {
    getIfProcessOnline,
    getProcessIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

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

const { extConfigs, extTypeMap } = getExtConfigsRelated();

const { td } = toRefs(getAllKfConfigData());
const tdIdList = computed(() => {
    return td.value.map(
        (item: KfConfig): string => `${item.group}_${item.name}`,
    );
});

const { processStatusData } = toRefs(getProcessStatusDetailData());

const { allProcessOnline, handleSwitchAllProcessStatus } = useSwitchAllConfig(
    td,
    processStatusData,
);

const { searchKeyword, tableData } = useTableSearchKeyword<KfConfig>(td, [
    'group',
    'name',
]);

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

function getStateStatusName(processId: string): ProcessStatusTypes {
    processId;
    return 'Unknown';
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
                    <template v-else-if="column.dataIndex === 'stateStatus'">
                        <KfProcessStatus
                            :status-name="
                                getStateStatusName(
                                    getProcessIdByKfLocation(record),
                                )
                            "
                        ></KfProcessStatus>
                    </template>
                    <template v-else-if="column.dataIndex === 'group'">
                        <a-tag :color="getExtColor(extTypeMap, record.group)">
                            {{ record.group }}
                        </a-tag>
                    </template>
                    <template v-else-if="column.dataIndex === 'processStatus'">
                        <a-switch
                            size="small"
                            :checked="
                                getIfProcessOnline(
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
                            <SettingOutlined style="font-size: 12px" />
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
            @confirm="handleSelectedSource"
        ></KfSetSourceModal>
        <KfSetByConfigModal
            v-if="setTdModalVisible"
            v-model:visible="setTdModalVisible"
            :payload="setTdConfigPayload"
            :primaryKeyCompareTarget="tdIdList"
            :primaryKeyCompareExtra="currentSelectedSourceId"
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
